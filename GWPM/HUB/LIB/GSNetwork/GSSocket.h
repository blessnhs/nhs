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

	BOOL			Listen(USHORT port, INT backLog);
	BOOL			Accept(SOCKET listenSocket);
	BOOL			Accept2(SOCKET listenSocket);



	BOOL			GetLocalIP(std::string& ipaddress,int &port);
	USHORT			GetLocalPort(VOID);
	
	BOOL			Connect(LPSTR address, USHORT port);
	SOCKET			Connect2(LPSTR address, USHORT port);

	SOCKET			GetSocket(VOID);
	BOOL RegisterIocp(GSNetwork::GSIocp::GSIocp& iocp, ULONG_PTR completionKey);
	
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, char *Info);
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, DWORD Info);

	BYTE *GetBuffer();
};

}	}