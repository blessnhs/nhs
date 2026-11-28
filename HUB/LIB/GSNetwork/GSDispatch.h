#pragma once

#include "GSTimerJob.h"

namespace GSFrames	{	namespace GSDispatch	{

typedef GSFrames::GSTimerJob::GSTimerJob	GSTimerJob;

/// @brief 완료 포트에 등록할 작업의 컨텍스트입니다.
struct _DispatchJob : public OVERLAPPED
{
	GSTimerJob  Job;
} ;

/// @brief 작업을 완료 포트에 등록하고 전용 워커에서 실행하는 디스패처입니다.
class GSDispatch : public GSGuard<GSDispatch>
{
public:
	/// @brief 디스패처의 초기 상태를 설정합니다.
	GSDispatch(void);
	/// @brief 디스패처 객체를 파괴합니다.
	~GSDispatch(void);

	HANDLE				m_Handle;
	DWORD				m_WTCnt;

	std::vector<HANDLE>	m_WTVector;

private:
	/// @brief 완료 포트에서 전달된 실행 작업을 처리합니다.
	VOID		OnExecuteCallback(VOID *object);

public:
	/// @brief 작업의 소유권을 넘겨받아 완료 포트에 등록합니다. 등록이 실패하면 즉시 작업을 해제합니다.
	VOID		Exe(_DispatchJob *pJob);

public:
	/// @brief 작업 처리에 사용할 완료 포트와 워커를 시작합니다.
	BOOL		Initialize(VOID);
	/// @brief 디스패처 워커와 완료 포트를 정리합니다.
	BOOL		Termination(VOID);
	
	/// @brief 완료 포트에서 작업을 꺼내 실행 콜백에 넘깁니다.
	VOID		WorkerThread2(VOID);
};

}	}