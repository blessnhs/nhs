#include "StdAfx.h"
#include "GSIocp.h"
#include "GSSocket.h"
#include "GSClient.h"

namespace GSNetwork	{	namespace GSIocp	{

typedef GSNetwork::GSSocket::GSSocket GSSocket;

DWORD WINAPI WorkerThread(LPVOID parameter)
{
	GSIocp *Owner = (GSIocp*) parameter;	
	Owner->WorkerThread();

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
		m_WTVector.push_back(WorkerThread);

		WaitForSingleObject(m_BeginEvtHandle, INFINITE);
	}

	return TRUE;
}

BOOL GSIocp::Termination(VOID)
{
	for (DWORD i=0;i<m_WTVector.size();i++)
		PostQueuedCompletionStatus(m_Handle, 0, 0, NULL);

	for (DWORD i=0;i<m_WTVector.size();i++)
	{
		WaitForSingleObject(m_WTVector[i], INFINITE);

		CloseHandle(m_WTVector[i]);
	}

	if (m_Handle)
		CloseHandle(m_Handle);

	m_WTVector.clear();

	if (m_BeginEvtHandle)
		CloseHandle(m_BeginEvtHandle);

	return TRUE;
}


BOOL GSIocp::RegIocpHandler(SOCKET socket, ULONG_PTR completionKey)
{
	if (!socket || !completionKey)
		return FALSE;

	// [LOCKFIX] 반환값을 m_Handle에 다시 대입하지 않는다.
	// 이전 코드는 여러 IOCP 스레드에서 동시에 m_Handle을 덮어썼고, 한 번이라도 실패하면(NULL)
	// 모든 소켓이 쓰는 IOCP 핸들이 NULL로 바뀌는 문제가 있었다.
	HANDLE Result = CreateIoCompletionPort((HANDLE) socket, m_Handle, completionKey, 0);

	if (!Result)
		return FALSE;

	return TRUE;
}

VOID GSIocp::WorkerThread()
{
	BOOL			Successed				= FALSE;
	DWORD			NumberOfByteTransfered	= 0;
	VOID			*CompletionKey			= NULL;
	OVERLAPPED		*Overlapped				= NULL;
	OVERLAPPED_EX	*OverlappedEx			= NULL;
	boost::shared_ptr<GSClient>			Object;

	// [LOCKFIX] 시작 알림은 한 번만 (이전: 완료 통지마다 SetEvent 시스템 호출)
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

			if (!CompletionKey)
			{
				// [LOCKFIX] Termination()이 PostQueuedCompletionStatus(m_Handle, 0, 0, NULL)로 보낸 종료 신호.
				// 이전에는 continue만 해서 워커가 끝나지 않았고 Termination()의 WaitForSingleObject가 영원히 대기했다.
				if (Overlapped == NULL)
					return;
				continue;
			}

			if(Overlapped == NULL) continue;

	
			boost::shared_ptr<OVERLAPPED_EX> OverlappedEx((OVERLAPPED_EX*)Overlapped);

	
			if(Overlapped == NULL && Successed == FALSE)
			{
				if(Object != NULL)
				{
					OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
				}
				continue;
			}

			if(Overlapped == NULL) continue;

			if(Successed == FALSE && Overlapped != NULL && Object != NULL)
			 {
				DWORD dwError = GetLastError();
				if(dwError==ERROR_OPERATION_ABORTED || dwError==ERROR_NETNAME_DELETED)
				{
				//	if(pCommon->GetSocketStatus() != STATUS_DISCONNECTED)
					if(Object->GetConnected() ==  TRUE)
					{
						OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
						continue;
					}
				}
				
				OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
				continue;
			}

			if (!Successed || (Successed && !NumberOfByteTransfered))
			{
				if (OverlappedEx->IoType == IO_ACCEPT)
					OnConnected(OverlappedEx->ObjectId);
				else
				{
					OnDisconnected2(OverlappedEx->ObjectId, OverlappedEx->IoType);
				}

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


		TCHAR szExeFileName[MAX_PATH];
		GetModuleFileName(NULL, szExeFileName, MAX_PATH);

//		printf("close call %s\n", szExeFileName);

		closesocket(client);
}


}	}