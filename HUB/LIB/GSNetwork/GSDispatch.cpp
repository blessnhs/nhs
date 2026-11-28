#include "StdAfx.h"
#include "GSDispatch.h"
#include <memory>

namespace GSFrames	{	namespace GSDispatch	{

DWORD WINAPI WorkerThread2(LPVOID parameter)
{
	GSDispatch *Owner = (GSDispatch*) parameter;
	Owner->WorkerThread2();

	return 0;
}

GSDispatch::GSDispatch(VOID)
{
	m_Handle			= NULL;
	m_WTCnt	= 0;
}

GSDispatch::~GSDispatch(VOID)
{
}

VOID GSDispatch::OnExecuteCallback(VOID *object)
{
	_DispatchJob *pJob = (_DispatchJob * )object;

	if (!pJob)
		return;

	std::unique_ptr<_DispatchJob> job(pJob);
	try
	{
		job->Job.Func((LPVOID)&job->Job);
	}
	catch (const std::exception& error)
	{
		SYSLOG().Write("[Dispatch worker] Exception detail=%.512s\n", error.what());
	}
	catch (...)
	{
		SYSLOG().Write("[Dispatch worker] Unknown exception\n");
	}
}

VOID GSDispatch::Exe(_DispatchJob *pJob)
{
	if (!PostQueuedCompletionStatus(m_Handle, 0, 0, pJob))
		delete pJob;
}

BOOL GSDispatch::Initialize(VOID)
{
	m_Handle			= NULL;

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	m_WTCnt	= SystemInfo.dwNumberOfProcessors * 1;
	m_Handle			= CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (!m_Handle)
		return FALSE;

	for (DWORD i=0;i<m_WTCnt;i++)
	{
		HANDLE WorkerThread = CreateThread(NULL, 0, GSFrames::GSDispatch::WorkerThread2, this, 0, NULL);
		if (!WorkerThread)
		{
			Termination();
			return FALSE;
		}
		m_WTVector.push_back(WorkerThread);
	}

	return TRUE;
}

BOOL GSDispatch::Termination(VOID)
{
	bool portClosed = false;
	for (DWORD i=0;i<m_WTVector.size();i++)
	{
		if (!PostQueuedCompletionStatus(m_Handle, 0, 0, NULL))
		{
			CloseHandle(m_Handle);
			portClosed = true;
			break;
		}
	}

	for (DWORD i=0;i<m_WTVector.size();i++)
	{
		WaitForSingleObject(m_WTVector[i], INFINITE);

		CloseHandle(m_WTVector[i]);
	}

	if (m_Handle && !portClosed)
		CloseHandle(m_Handle);
	m_Handle = NULL;

	m_WTVector.clear();

	return TRUE;
}
VOID GSDispatch::WorkerThread2(VOID)
{
	DWORD			NumberOfByteTransfered	= 0;
	VOID			*CompletionKey			= NULL;
	OVERLAPPED		*Overlapped				= NULL;

	while (TRUE)
	{
		GetQueuedCompletionStatus(
			m_Handle,
			&NumberOfByteTransfered,
			(PULONG_PTR) &CompletionKey,
			&Overlapped,
			INFINITE);

		if (!Overlapped)
			break;

		OnExecuteCallback(reinterpret_cast<_DispatchJob*>(Overlapped));
	}
}

}	}