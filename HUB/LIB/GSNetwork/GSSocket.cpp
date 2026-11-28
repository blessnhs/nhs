#include "StdAfx.h"
#include "GSSocket.h"
#include "GSIocp.h"
#include <WinSock2.h>
#include <mstcpip.h>

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
	if (Listen("0.0.0.0", port, backLog))
		return TRUE;
	const int error = WSAGetLastError();
	Termination();
	WSASetLastError(error);
	return FALSE;
}

BOOL GSSocket::Listen(const char* address, USHORT port, INT backLog)
{
	CThreadSync Sync(*this);

	if (port <= 0 || backLog <= 0)
	{
		WSASetLastError(WSAEINVAL);
		return FALSE;
	}

	if (!m_Socket)
		return FALSE;

	SOCKADDR_IN ListenSocketInfo{};
	ListenSocketInfo.sin_family = AF_INET;
	ListenSocketInfo.sin_port = htons(port);
	if (!address || InetPtonA(AF_INET, address, &ListenSocketInfo.sin_addr) != 1)
	{
		WSASetLastError(WSAEINVAL);
		return FALSE;
	}

	const int exclusive = 1;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
		reinterpret_cast<const char*>(&exclusive), sizeof(exclusive)) == SOCKET_ERROR)
		return FALSE;

	if (::bind(m_Socket, (struct sockaddr*) &ListenSocketInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		return FALSE;

	if (listen(m_Socket, backLog) == SOCKET_ERROR)
		return FALSE;

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

BOOL GSSocket::UpdateAcceptContext(GSSocket& listener)
{
	CThreadSync sync(listener);
	return UpdateAcceptContext(listener.m_Socket);
}

BOOL GSSocket::UpdateAcceptContext(SOCKET listenSocket)
{
	CThreadSync lock(*this);
	if (!m_Socket || m_Socket == INVALID_SOCKET || !listenSocket || listenSocket == INVALID_SOCKET)
		return FALSE;

	// optval은 리슨 소켓 핸들의 주소 (Winsock 문서 규약)
	if (setsockopt(m_Socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<const char*>(&listenSocket), sizeof(listenSocket)) == SOCKET_ERROR)
		return FALSE;

	return TRUE;
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
		try
		{
			if (m_Iocp->SubmitWrite(m_Socket, std::move(operation), length, address != nullptr))
				return TRUE;
		}
		catch (...)
		{
			m_OLP_REMAIN_COUNT_SND.fetch_sub(1);
			throw;
		}
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

BOOL GSSocket::SetSocketOption(int socket_id, int keep_alive, int keep_idle, int keep_count, int keep_intvl)
{
	DWORD dwRet;
	tcp_keepalive tcpkl;
	tcpkl.onoff = 1;
	tcpkl.keepalivetime = 5000; //5초마다 신호를 보냄
	tcpkl.keepaliveinterval = 1000;	// keep alive 신호를 받고 응답이 없으면 1초마다 재전송

	WSAIoctl(socket_id, SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcp_keepalive), 0, 0, &dwRet, NULL, NULL);



	//onoff = 1, l_longer = 0 closesocket시 강제 종료 남은 데이터 버림 TIME_WAIT남지 않음
	//onoff = 1, l_long != 0 blocking방식 종료처리 (graceful shutdown)
	//onoff = 0 //정상적 종료처리 종료처리 시점을 알수가 없다

	//LINGER ling;
	//ling.l_onoff = 1;
	//ling.l_linger = 0;

	//int ret = setsockopt(socket_id, SOL_SOCKET, SO_LINGER, (const char FAR*) & ling, sizeof(ling));
	//if (ret == -1)
	//	printf("fail keep alive socket opt\n");

	//서버입장에서는 2분의 time_wait지속 시간을 갖는 리소스 낭비이며 장애 발생 원인이 된다. 따라서 time_wait이 없는 abortive shutdonw으로 longer를 사용해야한다.

	/*int ret = setsockopt(socket_id, SOL_SOCKET, SO_KEEPALIVE, (const char FAR*) & keep_alive, sizeof(keep_alive));
	if (ret == -1)
		printf("fail keep alive socket opt\n");

	ret = setsockopt(socket_id, IPPROTO_TCP, TCP_KEEPIDLE, (const char FAR*) & keep_idle, sizeof(keep_idle));
	if (ret == -1)
		printf("fail keep_idle socket opt\n");

	ret = setsockopt(socket_id, IPPROTO_TCP, TCP_KEEPCNT, (const char FAR*) & keep_count, sizeof(keep_count));
	if (ret == -1)
		printf("fail keep_count socket opt\n");

	ret = setsockopt(socket_id, IPPROTO_TCP, TCP_KEEPINTVL, (const char FAR*) & keep_intvl, sizeof(keep_intvl));
	if (ret == -1)
		printf("fail keep_intvl socket opt\n");*/

	return TRUE;
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

	OVERLAPPED_EX* m_Accept_OLP = new OVERLAPPED_EX;
	m_OLP_REMAIN_COUNT_ACC.fetch_add(1);
	m_Accept_OLP->IoType = IO_ACCEPT;
	m_Accept_OLP->ObjectId = m_ClientId;

	SetSocketOption(m_Socket);

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
		if (ErrorCode != ERROR_IO_PENDING)
		{
			SYSLOG().Write("[IOCP/accept] Submission failed client=%d wsa=%d\n", m_ClientId, ErrorCode);
			m_OLP_REMAIN_COUNT_ACC.fetch_sub(1);
			delete m_Accept_OLP;
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

	OVERLAPPED_EX* m_Accept_OLP = new OVERLAPPED_EX;
	m_OLP_REMAIN_COUNT_ACC.fetch_add(1);
	m_Accept_OLP->IoType = IO_ACCEPT;
	m_Accept_OLP->ObjectId = m_ClientId;

	SetSocketOption(m_Socket);

	if (!AcceptEx(listenSocket, 
		m_Socket, 
		m_ReceiveBuffer,
		0, 
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, 
		NULL, 
		&m_Accept_OLP->Overlapped))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			m_OLP_REMAIN_COUNT_ACC.fetch_sub(1);
			delete m_Accept_OLP;
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