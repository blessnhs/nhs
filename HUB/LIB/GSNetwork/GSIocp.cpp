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
	{
		SYSLOG().Write("[IOCP/startup] CreateIoCompletionPort failed win32=%lu\n", GetLastError());
		return FALSE;
	}

	m_BeginEvtHandle = CreateEvent(0, FALSE, FALSE, 0);
	if (m_BeginEvtHandle == NULL)
	{
		SYSLOG().Write("[IOCP/startup] CreateEvent failed win32=%lu\n", GetLastError());
		Termination();

		return FALSE;
	}

	for (DWORD i=0;i<m_WTCnt;i++)
	{
		HANDLE WorkerThread = CreateThread(NULL, 0, GSNetwork::GSIocp::WorkerThread, this, 0, NULL);
		if (!WorkerThread)
		{
			SYSLOG().Write("[IOCP/startup] CreateThread failed worker=%lu win32=%lu\n", i, GetLastError());
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

size_t GSIocp::GetPendingWriteCount()
{
	std::lock_guard<std::mutex> pending(m_PendingWritesLock);
	return m_PendingWrites.size();
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

	if (CreateIoCompletionPort((HANDLE)socket, m_Handle, completionKey, 0))
		return TRUE;

	SYSLOG().Write("[IOCP/register] Socket association failed socket=%llu win32=%lu\n",
		static_cast<unsigned long long>(socket), GetLastError());
	return FALSE;
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
	const int error = result == SOCKET_ERROR ? WSAGetLastError() : 0;
	if (result == 0 || error == WSA_IO_PENDING)
		return TRUE;

	SYSLOG().Write("[IOCP/send] Submission failed socket=%llu bytes=%lu wsa=%d\n",
		static_cast<unsigned long long>(socket), length, error);

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
		// [FIX2] 2026-11-27 예외 발생 시 어느 연결/어떤 I/O였는지 기록하고, 멈춘 연결을 정리하기 위한 정보
		int		ExceptionObjectId	= -1;
		DWORD	ExceptionIoType		= 0;
		int		CleanupObjectId		= -1;	// 성공 처리(OnRead/OnConnected) 도중일 때만 설정

		try
		{

			Successed = GetQueuedCompletionStatus(
				m_Handle,
				&NumberOfByteTransfered,
				(PULONG_PTR)&CompletionKey,
				&Overlapped,
				INFINITE);

			const DWORD completionError = Successed ? ERROR_SUCCESS : GetLastError();
			if (!Overlapped)
			{
				if (Successed && !CompletionKey)
					break;
				if (completionError == ERROR_ABANDONED_WAIT_0)
					break;
				if (completionError == ERROR_INVALID_HANDLE)
					break;
				continue;
			}

			std::unique_ptr<OVERLAPPED_EX> OverlappedEx(reinterpret_cast<OVERLAPPED_EX*>(Overlapped));
			ExceptionObjectId	= OverlappedEx->ObjectId;
			ExceptionIoType		= (DWORD)OverlappedEx->IoType;

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
				static std::atomic<unsigned long long> acceptFailures{0};
				static std::atomic<unsigned long long> readFailures{0};
				static std::atomic<unsigned long long> writeFailures{0};
				static std::atomic<unsigned long long> otherFailures{0};
				auto* failures = &otherFailures;
				const char* operation = "other";
				switch (OverlappedEx->IoType)
				{
				case IO_ACCEPT: failures = &acceptFailures; operation = "accept"; break;
				case IO_READ: failures = &readFailures; operation = "read"; break;
				case IO_WRITE: failures = &writeFailures; operation = "write"; break;
				}
				const auto total = failures->fetch_add(1, std::memory_order_relaxed) + 1;
				if (total <= 4 || total % 1024 == 0)
					SYSLOG().Write("[IOCP/completion-failure] ClientId=%d Operation=%s Error=%lu TotalForOperation=%llu\n",
						OverlappedEx->ObjectId, operation, completionError, total);
				OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
				continue;
			}

			if (OverlappedEx->IoType == IO_ACCEPT)
			{
				CleanupObjectId = OverlappedEx->ObjectId;
				OnConnected(OverlappedEx->ObjectId);
				CleanupObjectId = -1;
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

					// OnRead 안에서 예외가 나면 수신 재등록(InitializeReadForIocp)이 실행되지 않아
					// 수신도 종료 처리도 없는 "멈춘 연결"이 남는다 -> catch에서 정리
					CleanupObjectId = OverlappedEx->ObjectId;
					OnRead(OverlappedEx->ObjectId, NumberOfByteTransfered);
					CleanupObjectId = -1;
				}
				break;

			case IO_WRITE:
				OnWrote(OverlappedEx->ObjectId, NumberOfByteTransfered);
				break;
			}
		
		}
		catch (const std::exception& e)
		{
			SYSLOG().Write("IOCP WORKER THREAD EXCEPTION object %d io %lu : %s\n", ExceptionObjectId, ExceptionIoType, e.what());
			CleanupAfterWorkerException(CleanupObjectId);
		}
		catch (...)
		{
			// /EHsc 빌드에서는 C++ 예외만 여기로 온다 (접근 위반 같은 SEH 예외는 크래시 핸들러로 감)
			SYSLOG().Write("IOCP WORKER THREAD UNKNOWN EXCEPTION object %d io %lu\n", ExceptionObjectId, ExceptionIoType);
			CleanupAfterWorkerException(CleanupObjectId);
		}
	}
}

// [FIX2] 성공 처리(OnRead/OnConnected) 도중 예외가 나서 수신이 다시 걸리지 않은 연결을 강제로 종료 처리한다.
// 종료 처리 중에 또 예외가 나도 워커 스레드가 죽지 않도록 여기서 한 번 더 감싼다.
// (OnDisconnected2 처리 중 예외는 이미 종료 처리 경로이므로 다시 호출하지 않는다)
VOID GSIocp::CleanupAfterWorkerException(int ObjectId)
{
	if (ObjectId < 0)
		return;

	try
	{
		OnDisconnected(ObjectId, true);
	}
	catch (const std::exception& e)
	{
		SYSLOG().Write("IOCP WORKER cleanup exception object %d : %s\n", ObjectId, e.what());
	}
	catch (...)
	{
		SYSLOG().Write("IOCP WORKER cleanup unknown exception object %d\n", ObjectId);
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