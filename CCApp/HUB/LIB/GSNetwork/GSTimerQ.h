#pragma once

#include "GSTimerJob.h"


template <class T>
class GSGuard;

namespace GSFrames	{	namespace GSTimerQ	{

typedef GSFrames::GSTimerJob::GSTimerJob	GSTimerJob;

class GSTimerQ : public GSGuard<GSTimerQ>
{
public:
	typedef std::multimap<DWORD,GSTimerJob*>::iterator iterator;

public:
	GSTimerQ(void);
	virtual ~GSTimerQ(void);

	int Initialize(int frequence=1000);

	bool AddTimerJob(GSTimerJob *pJob);
	bool DelTimerJob(GSTimerJob *pJob);

	static VOID CALLBACK SendMessage(LPVOID arg);

	int GetSize() { return (int)m_TimeJobList.size(); }

	static VOID CALLBACK Expine(PVOID lpParam);

protected:
	
	HANDLE hTimer;
	HANDLE hTimerQueue;
	HANDLE gDoneEvent;

public:
	int    MaxThread;
	// [LOCKFIX] 사용하지 않던 CRITICAL_SECTION cs 제거
protected:
//	tbb::concurrent_queue<GSTimerJob*> m_TimeJobList;
	// [LOCKFIX] map -> multimap: 같은 만료 시각의 작업을 키 충돌 회피 루프 없이 그대로 넣을 수 있다
	std::multimap<DWORD,GSTimerJob*> m_TimeJobList;

	
};

}	}