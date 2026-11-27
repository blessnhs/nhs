#include "StdAfx.h"
#include "GSProactorImpl.h"
#include "GSExecuteArgument.h"

namespace GSFrames	{	namespace GSProactorImpl	{

GSProactorImpl::GSProactorImpl(void)
{
	InitializeSRWLock(&m_MapLock);
	m_hKillEvent	= NULL;
	m_InputJobEvt	= NULL;
	m_hThread		= NULL;
}


GSProactorImpl::~GSProactorImpl(void)
{
	for each (auto job in m_JobMap)
	{
		if (job.second != NULL)
			delete job.second;
	}

	for each (auto evt in m_ExecuteJobEvt)
	{
		if (evt.second != NULL)
			CloseHandle(evt.second);
	}
}

// [LOCKFIX] 2026-11-26 작업 큐 구조 변경
//  이전 문제
//   1) Register()(분배 스레드)와 Handle_Event()(실행 스레드)가 동시에
//      "m_JobMap에 없으면 new 해서 넣기"를 수행 -> 큐가 두 번 생성되어 하나가 덮어써지고
//      먼저 들어간 메시지가 유실 + 메모리 누수. (concurrent_unordered_map은 find-then-insert를 원자적으로 해주지 않음)
//   2) 큐가 비면 Sleep(10) 폴링. 입력 큐 -> 분배 스레드 -> 타입 큐 2단계라 메시지마다 최대 약 20ms 지연.
//  변경
//   - 타입별 큐와 세마포어는 GetJobQueue()에서 SRWLOCK 이중 확인으로 딱 한 번만 생성 (Activate에서 미리 생성)
//   - RegisterCommand()가 타입 큐에 바로 push (분배 스레드 제거, 생산자별 순서는 그대로 보존)
//   - 실행 스레드는 세마포어로 대기 (push 1회 = ReleaseSemaphore 1회)

Concurrency::concurrent_queue<IMessagePtr>* GSProactorImpl::GetJobQueue(int Type, HANDLE* Semaphore)
{
	// 빠른 경로: 이미 있으면 공유 락으로 조회
	AcquireSRWLockShared(&m_MapLock);
	auto qit = m_JobMap.find(Type);
	if (qit != m_JobMap.end() && qit->second != NULL)
	{
		Concurrency::concurrent_queue<IMessagePtr>* q = qit->second;
		if (Semaphore) *Semaphore = m_ExecuteJobEvt[Type];
		ReleaseSRWLockShared(&m_MapLock);
		return q;
	}
	ReleaseSRWLockShared(&m_MapLock);

	// 느린 경로: 배타 락 안에서 다시 확인 후 생성
	AcquireSRWLockExclusive(&m_MapLock);
	qit = m_JobMap.find(Type);
	if (qit == m_JobMap.end() || qit->second == NULL)
	{
		m_ExecuteJobEvt[Type] = CreateSemaphore(NULL, 0, MAXLONG, NULL);
		m_JobMap[Type] = new Concurrency::concurrent_queue<IMessagePtr>();
	}
	Concurrency::concurrent_queue<IMessagePtr>* q = m_JobMap[Type];
	if (Semaphore) *Semaphore = m_ExecuteJobEvt[Type];
	ReleaseSRWLockExclusive(&m_MapLock);

	return q;
}

bool GSProactorImpl::Create(int InputTheadCnt)
{
	m_InputThreadCnt = InputTheadCnt;

	m_hKillEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_InputJobEvt	= CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL == m_hKillEvent || NULL == m_InputJobEvt)
	{
		SYSLOG().Write("Create Kill Event failed (%d)\n", GetLastError());
		return 1;
	}

	// [LOCKFIX] 분배 스레드(DistributionThread)는 더 이상 만들지 않는다.
	// RegisterCommand()가 타입별 큐에 직접 넣는다.

	return TRUE;
}

bool GSProactorImpl::Activate(int Type,int TCnt)
{
	// 실행 스레드가 돌기 전에 큐를 만들어 둔다
	GetJobQueue(Type, NULL);

	unsigned int WorkerId = 0;
	for(int i=0;i<TCnt;i++)
	{
		// [LOCKFIX] 스레드마다 인자를 따로 할당 (이전: 하나를 여러 스레드가 공유하면서 m_Id를 덮어씀)
		GSExecuteArgument *argument = new GSExecuteArgument();
		argument->m_ExecuteType		= Type;
		argument->m_ProcactorImpl   = this;
		argument->m_Id = WorkerId = i;

		m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, ExecuteThread, (LPVOID)argument, 0, &WorkerId));
	}

	return TRUE;
}

bool GSProactorImpl::RegisterCommand(IMessagePtr msg)
{
	if (msg == NULL)
	{
		SYSLOG().Write("pJob is null");
		return FALSE;
	}

	return Register(msg->Type, msg);
}

bool GSProactorImpl::Remove()
{
	return false;
}

bool GSProactorImpl::Register(int Type,IMessagePtr Msg)
{
	HANDLE Semaphore = NULL;
	Concurrency::concurrent_queue<IMessagePtr>* q = GetJobQueue(Type, &Semaphore);

	q->push(Msg);

	if (Semaphore != NULL)
		ReleaseSemaphore(Semaphore, 1, NULL);

	return TRUE;
}

// [LOCKFIX] 더 이상 시작하지 않는다. 혹시 m_InputJobList에 남은 작업이 있으면 타입 큐로 옮기는 용도로만 남겨 둔다.
unsigned int __stdcall DistributionThread(LPVOID parameter)
{
	GSProactorImpl *Owner = (GSProactorImpl*) parameter;

	IMessagePtr pJob;
	while(Owner->m_InputJobList.try_pop(pJob))
	{
		if(pJob != NULL)
			Owner->Register(pJob->Type,pJob);
	}

	return 0;
}

unsigned int __stdcall ExecuteThread(LPVOID parameter)
{
	GSExecuteArgument *Owner = (GSExecuteArgument*) parameter;

	int ProcId = Owner->m_ExecuteType;
	int Id		= Owner->m_Id;

	while(TRUE)
	{
//		if( WaitForSingleObject( Owner->m_ProcactorImpl->m_hKillEvent, 1 ) == WAIT_OBJECT_0 ){ break; }

		Owner->m_ProcactorImpl->Handle_Event(ProcId);
	}

	delete Owner;

	return 0;
}

bool GSProactorImpl::Handle_Event(int ProcId)
{
	HANDLE Semaphore = NULL;
	Concurrency::concurrent_queue<IMessagePtr>* q = GetJobQueue(ProcId, &Semaphore);

	// 항상 세마포어를 먼저 소비한다 (push 1회 = 카운트 1). 카운트와 큐 길이가 어긋나지 않게 하기 위함.
	// 100ms 타임아웃은 안전장치: 시간 초과여도 큐를 한 번 더 확인한다.
	WaitForSingleObject(Semaphore, 100);

	IMessagePtr pJob;
	if (q->try_pop(pJob) == FALSE)
		return false;

	if(pJob != NULL)
		pJob->Execute(pJob->Message);
	else
		SYSLOG().Write("execute pjob is null");

	return true;
}

}	}
