#include "StdAfx.h"
#include "GSTimerQ.h"

namespace GSFrames	{	namespace GSTimerQ	{

GSTimerQ::GSTimerQ(void)
{
	SYSTEM_INFO SystemInfo; 
	GetSystemInfo(&SystemInfo);
	MaxThread = 256;//SystemInfo.dwNumberOfProcessors;
}

GSTimerQ::~GSTimerQ(void)
{
}

VOID GSTimerQ::SendMessage(LPVOID Arg)
{
	GSTimerQ *pTimer = (GSTimerQ *)Arg;

	CThreadSync Sync(pTimer);	// [LOCKFIX] static 함수라 this 대신 pTimer

	iterator iter = pTimer->m_TimeJobList.begin();
	GSTimerJob *pJob = NULL;
	DWORD Now = GetTickCount();		// [LOCKFIX] 루프마다 호출하지 않고 한 번만
	while(iter != pTimer->m_TimeJobList.end())
	{
		pJob = (GSTimerJob *)iter->second;
		if(pJob == NULL)
		{
			// [LOCKFIX] 이전: break -> 뒤에 있는 만료 작업들이 다음 틱으로 밀림
			iter = pTimer->m_TimeJobList.erase(iter);
			continue;
		}
		
		if(Now >= pJob->GetExpineTime())
		{
			if(pJob->Func == NULL)
			{
				iter = pTimer->m_TimeJobList.erase(iter);
				continue;	// [LOCKFIX] 이전: break
			}
			ULONG nFlags = WT_EXECUTEDEFAULT;
			WT_SET_MAX_THREADPOOL_THREADS(nFlags, pTimer->MaxThread);
			QueueUserWorkItem((LPTHREAD_START_ROUTINE)pJob->Func,(LPVOID)pJob,nFlags);

			iter = pTimer->m_TimeJobList.erase(iter);
		}
		else
			++iter;
	}
}

VOID GSTimerQ::Expine(PVOID Arg)
{
	GSTimerQ *pTimer = (GSTimerQ *)Arg;

	GSTimerQ::SendMessage((LPVOID)pTimer);
}


bool GSTimerQ::AddTimerJob(GSTimerJob *pJob)
{
	if(pJob == NULL) return false;

	CThreadSync Sync(this);

	// [LOCKFIX] 이전: 같은 키를 피하려고 락 안에서 find()를 최대 1만 번 반복
	m_TimeJobList.insert(std::make_pair(pJob->GetExpineTime(), pJob));

	return true;
}

bool GSTimerQ::DelTimerJob(GSTimerJob *pJob)
{
	if (pJob == NULL)
		return false;

	CThreadSync sync(this);

	iterator iter = m_TimeJobList.begin();

	while(iter != m_TimeJobList.end())
	{
		GSTimerJob *Job =  (GSTimerJob *)(iter->second);
		if(pJob == Job)
		{
			iter = m_TimeJobList.erase(iter);
		}
		else
			++iter;
	}
	
	return true;
}

int GSTimerQ::Initialize(int frequence)
{
	hTimer = NULL;
	hTimerQueue = NULL;
	int arg = 123;

	gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == gDoneEvent)
    {
        SYSLOG().Write("CreateEvent failed (%d)\n", GetLastError());
        return 1;
    }
	
	hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        SYSLOG().Write("CreateTimerQueue failed (%d)\n", GetLastError());
        return 2;
    }

    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue, 
            (WAITORTIMERCALLBACK)Expine, this , frequence, frequence, frequence))
    {
        SYSLOG().Write("CreateTimerQueueTimer failed (%d)\n", GetLastError());
        return 3;
    }
	return true;
}

}	}