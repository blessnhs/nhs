#include "StdAfx.h"
#include "GSSocket.h"
#include "GSIocp.h"
#include <WinSock2.h>

namespace GSNetwork	{ namespace GSSocket	{

GSSocket::GSSocket(VOID)
{
	m_OLP_REMAIN_COUNT_ACC = 0;
	m_OLP_REMAIN_COUNT_REC = 0;
	m_OLP_REMAIN_COUNT_SND = 0;

	memset(m_ReceiveBuffer, 0, sizeof(m_ReceiveBuffer));
	m_Socket							= NULL;

	m_bConnected = FALSE;

	m_ClientId = -1;
}

BYTE *GSSocket::GetBuffer()
{
	return m_ReceiveBuffer;
}

GSSocket::~GSSocket(VOID)
{
}

BOOL GSSocket::Initialize(VOID)
{
	CThreadSync Sync(*this);

	m_bConnected = FALSE;

	if (m_Socket)
		return FALSE;

	memset(m_ReceiveBuffer, 0, sizeof(m_ReceiveBuffer));

	m_Socket							= NULL;
	return TRUE;
}

BOOL GSSocket::Termination(VOID)
{
	CThreadSync Sync(*this);

	if (!m_Socket)
		return FALSE;

	closesocket(m_Socket);
	m_Iocp = nullptr;

	m_Socket					= NULL;
	return TRUE;
}

BOOL GSSocket::Listen(USHORT port, INT backLog)
{
	CThreadSync Sync(*this);

	if (port <= 0 || backLog <= 0)
		return FALSE;

	if (!m_Socket)
		return FALSE;

	SOCKADDR_IN ListenSocketInfo;

	ListenSocketInfo.sin_family				= AF_INET;
	ListenSocketInfo.sin_port				= htons(port);
	ListenSocketInfo.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);

	int option = 1;

	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

	if (::bind(m_Socket, (struct sockaddr*) &ListenSocketInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		Termination();

		return FALSE;
	}

	if (listen(m_Socket, backLog) == SOCKET_ERROR)
	{
		Termination();

		return FALSE;
	}

	LINGER Linger;
	Linger.l_onoff	= 1;
	Linger.l_linger = 0;

	if (setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char*) &Linger, sizeof(LINGER)) == SOCKET_ERROR)
	{
		Termination();

		return FALSE;
	}

	return TRUE;
}

SOCKET GSSocket::GetSocket(VOID)
{
	CThreadSync Sync(*this);
	return m_Socket; 
}

BOOL GSSocket::RegisterIocp(GSNetwork::GSIocp::GSIocp& iocp, ULONG_PTR completionKey)
{
	CThreadSync lock(*this);
	if (!m_Socket || !iocp.RegIocpHandler(m_Socket, completionKey))
		return FALSE;

	m_Iocp = &iocp;
	return TRUE;
}

BOOL GSSocket::SubmitWrite(std::unique_ptr<BYTE[]> data, DWORD length, const SOCKADDR_IN* address)
{
	CThreadSync lock(*this);
	if (!m_Socket || m_Socket == INVALID_SOCKET || !data || !length)
		return FALSE;

	auto operation = std::make_unique<OVERLAPPED_EX>();
	operation->IoType = IO_WRITE;
	operation->ObjectId = m_ClientId;
	operation->SendBuffer = data.release();
	if (address)
		operation->SendAddress = *address;

	if (m_Iocp)
	{
		m_OLP_REMAIN_COUNT_SND.fetch_add(1);
		if (m_Iocp->SubmitWrite(m_Socket, std::move(operation), length, address != nullptr))
			return TRUE;
		m_OLP_REMAIN_COUNT_SND.fetch_sub(1);
		return FALSE;
	}

	// GSClientModule uses WSAEventSelect and has no IOCP worker.
	const WSAEVENT completed = WSACreateEvent();
	if (completed == WSA_INVALID_EVENT)
		return FALSE;

	operation->Overlapped.hEvent =
		reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(completed) | 1);
	WSABUF buffer{length, reinterpret_cast<char*>(operation->SendBuffer)};
	DWORD transferred = 0;
	DWORD flags = 0;
	const int result = address
		? WSASendTo(m_Socket, &buffer, 1, &transferred, 0,
			reinterpret_cast<const SOCKADDR*>(&operation->SendAddress), sizeof(SOCKADDR_IN),
			&operation->Overlapped, nullptr)
		: WSASend(m_Socket, &buffer, 1, &transferred, 0, &operation->Overlapped, nullptr);
	const BOOL success = result == 0 ||
		(WSAGetLastError() == WSA_IO_PENDING &&
			WaitForSingleObject(completed, INFINITE) == WAIT_OBJECT_0 &&
			WSAGetOverlappedResult(m_Socket, &operation->Overlapped, &transferred, FALSE, &flags));
	WSACloseEvent(completed);
	return success && transferred == length;
}

SOCKET GSSocket::Connect2(LPSTR address, USHORT port)
{
	CThreadSync Sync(*this);

	if (!m_Socket)
		m_Socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);;

	SOCKADDR_IN RemoteAddressInfo;
	ZeroMemory(&RemoteAddressInfo, sizeof(RemoteAddressInfo));

	RemoteAddressInfo.sin_family = AF_INET;
	RemoteAddressInfo.sin_port = htons(port);
	RemoteAddressInfo.sin_addr.S_un.S_addr = inet_addr(address);




	if (m_Socket == INVALID_SOCKET)
	{
		Termination();

		return FALSE;
	}

	struct linger lingerOpt;
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof(lingerOpt));

	int opt = 1;
	//		if(setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (const char*)&opt, sizeof(opt))==SOCKET_ERROR)
	//			return false;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
		return false;
	struct sockaddr_in addr;
	memset((void*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(address);
	if (addr.sin_addr.s_addr == INADDR_NONE) {
		LPHOSTENT lpHosten = gethostbyname(address);
		if (lpHosten == NULL) return false;
		addr.sin_addr.s_addr = ((LPIN_ADDR)lpHosten->h_addr)->s_addr;
	}
	if (WSAConnect(m_Socket, (LPSOCKADDR)&RemoteAddressInfo, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	//if (connect(m_Socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return false;

	return m_Socket;
}

BOOL GSSocket::Accept(SOCKET listenSocket)
{
	CThreadSync Sync(*this);

	if (!listenSocket)
		return FALSE;

	if (m_Socket)
		return FALSE;

	m_Socket	= WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_Socket == INVALID_SOCKET)
	{
		Termination();

		return FALSE;
	}

	m_OLP_REMAIN_COUNT_ACC.fetch_add(1);

	OVERLAPPED_EX* m_Accept_OLP = new OVERLAPPED_EX;
	m_Accept_OLP->IoType = IO_ACCEPT;
	m_Accept_OLP->ObjectId = m_ClientId;

	//BOOL NoDelay = TRUE;
	//setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));

	if (!AcceptEx(listenSocket, 
		m_Socket, 
		m_ReceiveBuffer,
		0, 
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, 
		NULL, 
		&m_Accept_OLP->Overlapped))
	{
		int ErrorCode = WSAGetLastError();
		if (ErrorCode != ERROR_IO_PENDING && ErrorCode != WSAEWOULDBLOCK)
		{
			m_OLP_REMAIN_COUNT_ACC.fetch_sub(1);
			Termination();

			return FALSE;
		}
	}

	return TRUE;
}

BOOL GSSocket::Accept2(SOCKET listenSocket)
{
	CThreadSync Sync(*this);

	if (!listenSocket)
		return FALSE;

	if (m_Socket == INVALID_SOCKET)
	{
		Termination();

		return FALSE;
	}

	m_OLP_REMAIN_COUNT_ACC.fetch_add(1);

	OVERLAPPED_EX* m_Accept_OLP = new OVERLAPPED_EX;
	m_Accept_OLP->IoType = IO_ACCEPT;
	m_Accept_OLP->ObjectId = m_ClientId;

	//BOOL NoDelay = TRUE;
	//setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));

	if (!AcceptEx(listenSocket, 
		m_Socket, 
		m_ReceiveBuffer,
		0, 
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, 
		NULL, 
		&m_Accept_OLP->Overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			m_OLP_REMAIN_COUNT_ACC.fetch_sub(1);
			Termination();

			return FALSE;
		}
	}

	return TRUE;
}


BOOL GSSocket::Connect(LPSTR address, USHORT port)
{
	CThreadSync Sync(*this);

	if (!address || port <= 0)
		return FALSE;

	if (!m_Socket)
		return FALSE;

	SOCKADDR_IN RemoteAddressInfo;

	RemoteAddressInfo.sin_family			= AF_INET;
	RemoteAddressInfo.sin_port				= htons(port);
	RemoteAddressInfo.sin_addr.S_un.S_addr	= inet_addr(address);

	//inet_pton(AF_INET, address, &RemoteAddressInfo.sin_addr.S_un.S_addr);

	if (WSAConnect(m_Socket, (LPSOCKADDR) &RemoteAddressInfo, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		int returnCode  = WSAGetLastError();
		if (returnCode != WSAEWOULDBLOCK)
		{
			Termination();

			return FALSE;
		}
	}

	return TRUE;
}

BOOL GSSocket::GetLocalIP(std::string& ipaddress, int& port)
{
	CThreadSync Sync(*this);

	if(!m_Socket)
		return FALSE;


	sockaddr_in* Local = NULL;
	INT				LocalLength = 0;

	sockaddr_in* Remote = NULL;
	INT				RemoteLength = 0;

	GetAcceptExSockaddrs(m_ReceiveBuffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr**)&Local,
		&LocalLength,
		(sockaddr**)&Remote,
		&RemoteLength);

//	char remoteAddress[64];

//	_tcscpy(remoteAddress, (LPTSTR)(inet_ntoa(Remote->sin_addr)));

	ipaddress = (LPTSTR)(inet_ntoa(Remote->sin_addr));

	port = ntohs(Remote->sin_port);


	return FALSE;
}

USHORT GSSocket::GetLocalPort(VOID)
{
	CThreadSync Sync(*this);

	if (!m_Socket)
		return 0;

	SOCKADDR_IN Addr;
	ZeroMemory(&Addr, sizeof(Addr));

	INT AddrLength = sizeof(Addr);
	if (getsockname(m_Socket, (sockaddr*) &Addr, &AddrLength) != SOCKET_ERROR)
		return ntohs(Addr.sin_port);

	return 0;
}

long GSSocket::ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, char *Info)
{
	PEXCEPTION_RECORD pRecord = pExceptionInfo->ExceptionRecord;


	char strExceptionDescription[512];
	sprintf_s( strExceptionDescription, "The system takes place the exception in the 0x%p while run function of %s,error code: 0x%08X",
									   (pRecord->ExceptionAddress), 
									   Info,
		                               pRecord->ExceptionCode);

	return EXCEPTION_EXECUTE_HANDLER;
}

long GSSocket::ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, DWORD Info)
{
	PEXCEPTION_RECORD pRecord = pExceptionInfo->ExceptionRecord;


	char strExceptionDescription[512];
	sprintf_s( strExceptionDescription, "The system takes place the exception in the 0x%p while run function of 0x%08X,error code: 0x%08X",
									   (pRecord->ExceptionAddress), 
		                               Info,
		                               pRecord->ExceptionCode);

	return EXCEPTION_EXECUTE_HANDLER;
}


}	}