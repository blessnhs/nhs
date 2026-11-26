#include "StdAfx.h"
#include "GSIocp.h"
#include "GSSocket.h"
#include "GSClient.h"
#include <chrono>

namespace GSNetwork	{	namespace GSIocp	{

typedef GSNetwork::GSSocket::GSSocket GSSocket;

namespace
{
	thread_local GSIocp* currentIocpWorker = nullptr;
}

DWORD WINAPI WorkerThread(LPVOID parameter)
{
	GSIocp *Owner = (GSIocp*) parameter;	
	currentIocpWorker = Owner;
	Owner->WorkerThread();
	currentIocpWorker = nullptr;

	return 0;
}

GSIocp::GSIocp(VOID)
{
	m_Handle			= NULL;
	m_WTCnt	= 0;

	m_BeginEvtHandle	= NULL;
}

GSIocp::~GSIocp(VOID)
{
}

BOOL GSIocp::Initialize(VOID)
{
	m_Handle			= NULL;

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	m_WTCnt	= SystemInfo.dwNumberOfProcessors * 2;
	m_Handle			= CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (!m_Handle)
		return FALSE;

	m_BeginEvtHandle = CreateEvent(0, FALSE, FALSE, 0);
	if (m_BeginEvtHandle == NULL)
	{
		Termination();

		return FALSE;
	}

	for (DWORD i=0;i<m_WTCnt;i++)
	{
		HANDLE WorkerThread = CreateThread(NULL, 0, GSNetwork::GSIocp::WorkerThread, this, 0, NULL);
		if (!WorkerThread)
		{
			Termination();
			return FALSE;
		}

		m_WTVector.push_back(WorkerThread);
		WaitForSingleObject(m_BeginEvtHandle, INFINITE);
	}

	{
		std::unique_lock<std::shared_timed_mutex> lock(m_SubmissionLock);
		m_AcceptingWrites = true;
	}
	return TRUE;
}

BOOL GSIocp::Termination(VOID)
{
	// The owning thread must stop the port after the callback returns.
	if (currentIocpWorker == this)
		return FALSE;

	DWORD cancellationError = ERROR_SUCCESS;
	{
		std::unique_lock<std::shared_timed_mutex> submissions(m_SubmissionLock);
		m_AcceptingWrites = false;
		std::lock_guard<std::mutex> pending(m_PendingWritesLock);
		for (const auto& entry : m_PendingWrites)
		{
			if (!CancelIoEx(reinterpret_cast<HANDLE>(entry.second), entry.first))
			{
				const DWORD error = GetLastError();
				if (error != ERROR_NOT_FOUND && cancellationError == ERROR_SUCCESS)
					cancellationError = error;
			}
		}
	}
	if (cancellationError != ERROR_SUCCESS)
		SYSLOG().Write("IOCP send cancellation returned error %lu\n", cancellationError);

	{
		std::unique_lock<std::mutex> pending(m_PendingWritesLock);
		while (!m_WritesDrained.wait_for(pending, std::chrono::seconds(5),
			[this] { return m_PendingWrites.empty(); }))
		{
			const size_t outstanding = m_PendingWrites.size();
			pending.unlock();
			SYSLOG().Write("IOCP shutdown waiting for %zu send completions\n", outstanding);
			pending.lock();
		}
	}

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

	if (m_BeginEvtHandle)
		CloseHandle(m_BeginEvtHandle);
	m_BeginEvtHandle = NULL;

	return TRUE;
}


BOOL GSIocp::RegIocpHandler(SOCKET socket, ULONG_PTR completionKey)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_SubmissionLock);
	if (!m_AcceptingWrites || !m_Handle || socket == INVALID_SOCKET || !completionKey)
		return FALSE;

	return CreateIoCompletionPort((HANDLE)socket, m_Handle, completionKey, 0) != NULL;
}

BOOL GSIocp::SubmitWrite(SOCKET socket, std::unique_ptr<OVERLAPPED_EX> operation, DWORD length, bool addressed)
{
	std::shared_lock<std::shared_timed_mutex> submissions(m_SubmissionLock);
	if (!m_AcceptingWrites)
		return FALSE;

	WSABUF buffer{length, reinterpret_cast<char*>(operation->SendBuffer)};
	OVERLAPPED* overlapped = &operation->Overlapped;
	const SOCKADDR* address = reinterpret_cast<const SOCKADDR*>(&operation->SendAddress);
	{
		std::lock_guard<std::mutex> pending(m_PendingWritesLock);
		m_PendingWrites.emplace(overlapped, socket);
	}
	// A worker can dequeue completion before the submitting thread returns.
	operation.release();

	const int result = addressed
		? WSASendTo(socket, &buffer, 1, nullptr, 0, address, sizeof(SOCKADDR_IN), overlapped, nullptr)
		: WSASend(socket, &buffer, 1, nullptr, 0, overlapped, nullptr);
	if (result == 0 || WSAGetLastError() == WSA_IO_PENDING)
		return TRUE;

	// Winsock sends no completion for a synchronous submission failure.
	{
		std::lock_guard<std::mutex> pending(m_PendingWritesLock);
		m_PendingWrites.erase(overlapped);
	}
	operation.reset(reinterpret_cast<OVERLAPPED_EX*>(overlapped));
	return FALSE;
}

VOID GSIocp::WorkerThread()
{
	BOOL			Successed				= FALSE;
	DWORD			NumberOfByteTransfered	= 0;
	VOID			*CompletionKey			= NULL;
	OVERLAPPED		*Overlapped				= NULL;
	SetEvent(m_BeginEvtHandle);

	while (TRUE)
	{
		try
		{

			Successed = GetQueuedCompletionStatus(
				m_Handle,
				&NumberOfByteTransfered,
				(PULONG_PTR)&CompletionKey,
				&Overlapped,
				INFINITE);

			if (!Overlapped)
			{
				if (Successed && !CompletionKey)
					break;
				if (!Successed && GetLastError() == ERROR_ABANDONED_WAIT_0)
					break;
				if (!Successed && GetLastError() == ERROR_INVALID_HANDLE)
					break;
				continue;
			}

			std::unique_ptr<OVERLAPPED_EX> OverlappedEx(reinterpret_cast<OVERLAPPED_EX*>(Overlapped));
			if (OverlappedEx->IoType == IO_WRITE)
			{
				std::lock_guard<std::mutex> pending(m_PendingWritesLock);
				m_PendingWrites.erase(Overlapped);
				if (m_PendingWrites.empty())
					m_WritesDrained.notify_all();
			}

			if (!CompletionKey)
				continue;
			if (!Successed)
			{
				OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
				continue;
			}

			if (OverlappedEx->IoType == IO_ACCEPT)
			{
				OnConnected(OverlappedEx->ObjectId);
				continue;
			}

			switch (OverlappedEx->IoType)
			{
			case IO_READ:
				{
					if(NumberOfByteTransfered == 0)
					{
						OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
						break;
					}
					OnRead(OverlappedEx->ObjectId, NumberOfByteTransfered);
				}
				break;

			case IO_WRITE:
				OnWrote(OverlappedEx->ObjectId, NumberOfByteTransfered);
				break;
			}
		
		}
		catch (...)
        {
			/*if(Object != NULL)
			{
				OnDisconnected(Object);
			}*/
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION DETECTED\n");
		}
	}
}

LONG GSIocp::ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, LPCSTR szPosition)
{
	PEXCEPTION_RECORD	lpExceptionRecord		= pExceptionInfo->ExceptionRecord;
	DWORD				dwExceptionCode			= lpExceptionRecord->ExceptionCode;
	LPVOID				lpExceptionAddress		= lpExceptionRecord->ExceptionAddress;
	TCHAR				szExceptionInfo[256]	= { _T( "" ) };
	wsprintf( szExceptionInfo, _T( "Code:0x%08X  Address:0x%p at %s" ), \
              dwExceptionCode, lpExceptionAddress, szPosition );
	LPTSTR lpExceptionInfo = szExceptionInfo + _tcslen( szExceptionInfo );
	if ( dwExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	{
		LPSTR lp = lpExceptionRecord->ExceptionInformation[0] ? "write":"read";
		wsprintf( lpExceptionInfo, TEXT("Attempt to %s data at 0x%p.\r\n"), lp, 
                  lpExceptionRecord->ExceptionInformation[1] );
	}

	return ( EXCEPTION_EXECUTE_HANDLER );
}

VOID GSIocp::Close(SOCKET client)
{
		//iocp에서 closesocket하면 안된다. 

	/*	OVERLAPPED_EX* ex = new OVERLAPPED_EX;

		ex->ObjectId = client;
		ex->IoType = IO_READ;

		PostQueuedCompletionStatus(m_Handle, 0, (ULONG_PTR)ex, (LPOVERLAPPED)ex);*/

		closesocket(client);
}


}	}