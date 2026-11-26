#pragma once

#include "IHandler.h"
#include "./IMessage.h"
#include "ICommand.h"

#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>

namespace GSFrames	{	namespace GSProactorImpl	{

class GSProactorImpl
{
public:
	GSProactorImpl(void);
	~GSProactorImpl(void);

	virtual bool Create(int InputTheadCnt);
	virtual bool Activate(int Type,int TCnt);
	virtual bool RegisterCommand(IMessagePtr msg);
	
	virtual bool Register(int Type,IMessagePtr Msg);
	virtual bool Remove();
	virtual bool Handle_Event(int ProcId);

protected:
	// [LOCKFIX] 타입별 작업 큐와 세마포어를 한 번만 생성해서 돌려준다 (생성 경쟁 방지)
	Concurrency::concurrent_queue<IMessagePtr>* GetJobQueue(int Type, HANDLE* Semaphore);
	SRWLOCK				m_MapLock;

protected:
	int				   m_InputThreadCnt;
	

	//잡 등록 
	Concurrency::concurrent_queue<IMessagePtr> m_InputJobList;

	HANDLE			   m_hKillEvent;	
	HANDLE			   m_hThread;

	concurrency::concurrent_unordered_map<int, Concurrency::concurrent_queue<IMessagePtr>*> m_JobMap;
	concurrency::concurrent_unordered_map<int, HANDLE > m_ExecuteJobEvt;
	GSProactorImpl		*m_ProactorImpl;

	//등록된 잡에서 개별 처리 map으로 넣어준다.
	friend unsigned int __stdcall DistributionThread(LPVOID parameter);

	//id에 해당하는 map id에서 데이터를 꺼내 처리한다.
	friend unsigned int __stdcall ExecuteThread(LPVOID parameter);

	HANDLE				m_InputJobEvt;
};

}	}

