#pragma once

#include "GSSocketBase.h"
#include "Lock.h"

namespace GSNetwork { namespace GSIocp { class GSIocp; } }

namespace GSNetwork	{ namespace GSSocket	{

// Socket Status
#define STATUS_DISCONNECTED		0x00
#define STATUS_CONNECTED		0x01
#define STATUS_LISTENED			0x02

class GSSocket
{
private:
	CGSLock m_SocketLock;
	GSNetwork::GSIocp::GSIocp* m_Iocp = nullptr;

protected:
	BOOL SubmitWrite(std::unique_ptr<BYTE[]> data, DWORD length, const SOCKADDR_IN* address = nullptr);

	class CThreadSync
	{
	public:
		explicit CThreadSync(GSSocket& owner) : m_Lock(owner.m_SocketLock)
		{
			m_Lock.Enter();
		}

		~CThreadSync()
		{
			m_Lock.Leave();
		}

		CThreadSync(const CThreadSync&) = delete;
		CThreadSync& operator=(const CThreadSync&) = delete;

	private:
		CGSLock& m_Lock;
	};

public:
	GSSocket(const GSSocket&) = delete;
	GSSocket& operator=(const GSSocket&) = delete;
	GSSocket(VOID);
	virtual ~GSSocket(VOID);

public:

	std::atomic<int>		m_OLP_REMAIN_COUNT_ACC;
	std::atomic<int>		m_OLP_REMAIN_COUNT_REC;
	std::atomic<int>		m_OLP_REMAIN_COUNT_SND;
		
	int				m_ClientId;

protected:
	
					//wsarecvieve에서 이번에 전송받은 패킷 버퍼
	BYTE			m_ReceiveBuffer[MAX_BUFFER_LENGTH];

	SOCKET			m_Socket;

	BOOL			m_bConnected;

public:
	BOOL			Initialize(VOID);
	BOOL			Termination(VOID);

	// [FIX3] 2026-11-27 소켓 락 아래에서 소켓만 닫는다 (closesocket + m_Socket = NULL).
	// 외부에서 GetSocket()으로 핸들을 꺼내 closesocket 하면 안 된다:
	// 닫힌 핸들 값을 OS가 새 소켓에 다시 배정하면, 아직 그 값을 들고 있는 다른 스레드가
	// 엉뚱한 연결로 송수신하게 된다. 이 함수는 송수신 경로와 같은 소켓 락을 잡고 닫으므로 안전하다.
	// GSPacketTCP::Termination과 달리 수신 버퍼 등 패킷 상태는 건드리지 않는다
	// (다른 워커가 같은 연결의 수신을 처리 중이어도 충돌하지 않음).
	BOOL			CloseSocket(VOID) { return GSSocket::Termination(); }

	BOOL			Listen(USHORT port, INT backLog);
	BOOL			Accept(SOCKET listenSocket);
	BOOL			Accept2(SOCKET listenSocket);

	BOOL			SetSocketOption(int socket_id,int keep_alive = 40, int keep_idle = 600, int keep_count  = 10, int keep_intvl = 8);


	BOOL			GetLocalIP(std::string& ipaddress,int &port);
	USHORT			GetLocalPort(VOID);
	
	BOOL			Connect(LPSTR address, USHORT port);
	SOCKET			Connect2(LPSTR address, USHORT port);

	SOCKET			GetSocket(VOID);
	BOOL RegisterIocp(GSNetwork::GSIocp::GSIocp& iocp, ULONG_PTR completionKey);

	// [FIX4] 2026-11-27 AcceptEx로 받은 소켓에 리슨 소켓의 속성을 상속시킨다 (SO_UPDATE_ACCEPT_CONTEXT).
	// 설정하지 않으면 getpeername / getsockname / shutdown / 일부 setsockopt가 실패한다.
	BOOL UpdateAcceptContext(SOCKET listenSocket);
	
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, char *Info);
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, DWORD Info);

	BYTE *GetBuffer();
};

}	}