#pragma once

#include "GSSocketBase.h"
#include "Lock.h"

namespace GSNetwork { namespace GSIocp { class GSIocp; } }

namespace GSNetwork	{ namespace GSSocket	{

// Socket Status
#define STATUS_DISCONNECTED		0x00
#define STATUS_CONNECTED		0x01
#define STATUS_LISTENED			0x02

/// @brief Winsock 핸들과 소켓 잠금, IOCP 연결 및 공통 송신 경로를 관리합니다.
class GSSocket
{
private:
	CGSLock m_SocketLock;
	GSNetwork::GSIocp::GSIocp* m_Iocp = nullptr;

protected:
	/// @brief 송신 버퍼 소유권을 받아 IOCP 송신 또는 이벤트 기반 송신 경로에 전달합니다.
	BOOL SubmitWrite(std::unique_ptr<BYTE[]> data, DWORD length, const SOCKADDR_IN* address = nullptr);

	/// @brief 소켓의 CriticalSection을 스코프 수명 동안 보유하는 잠금 가드입니다.
	class CThreadSync
	{
	public:
		/// @brief 대상 소켓의 CriticalSection을 획득합니다.
		explicit CThreadSync(GSSocket& owner) : m_Lock(owner.m_SocketLock)
		{
			m_Lock.Enter();
		}

		/// @brief 획득한 소켓 CriticalSection을 해제합니다.
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
	/// @brief 소켓과 I/O 카운터의 초기 상태를 설정합니다.
	GSSocket(VOID);
	/// @brief 기반 소켓 객체를 파괴합니다.
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
	/// @brief 수신 버퍼와 소켓 기본 상태를 초기화합니다.
	BOOL			Initialize(VOID);
	/// @brief 소켓 잠금 아래 핸들을 닫고 IOCP 연결 참조를 해제합니다.
	BOOL			Termination(VOID);

	// [FIX3] 2026-11-27 소켓 락 아래에서 소켓만 닫는다 (closesocket + m_Socket = NULL).
	// 외부에서 GetSocket()으로 핸들을 꺼내 closesocket 하면 안 된다:
	// 닫힌 핸들 값을 OS가 새 소켓에 다시 배정하면, 아직 그 값을 들고 있는 다른 스레드가
	// 엉뚱한 연결로 송수신하게 된다. 이 함수는 송수신 경로와 같은 소켓 락을 잡고 닫으므로 안전하다.
	// GSPacketTCP::Termination과 달리 수신 버퍼 등 패킷 상태는 건드리지 않는다
	// (다른 워커가 같은 연결의 수신을 처리 중이어도 충돌하지 않음).
	/// @brief 소켓 잠금 아래 핸들을 닫습니다. 파생 패킷 버퍼는 그대로 유지됩니다.
	BOOL			CloseSocket(VOID) { return GSSocket::Termination(); }

	/// @brief 로컬 포트에 TCP 소켓을 바인딩하고 지정 backlog로 수신 대기를 시작합니다.
	BOOL			Listen(USHORT port, INT backLog);
	/// @brief 지정한 IPv4 주소와 TCP 포트에서 접속 대기를 시작합니다.
	BOOL Listen(const char* address, USHORT port, INT backLog);
	/// @brief 리스닝 소켓에 대해 AcceptEx를 등록합니다.
	BOOL			Accept(SOCKET listenSocket);
	/// @brief 리스닝 소켓에 대해 대체 Accept 등록 경로를 실행합니다.
	BOOL			Accept2(SOCKET listenSocket);

	/// @brief 소켓에 TCP keep-alive 옵션 설정을 시도합니다.
	BOOL			SetSocketOption(int socket_id,int keep_alive = 40, int keep_idle = 600, int keep_count  = 10, int keep_intvl = 8);


	/// @brief 소켓의 로컬 IPv4 주소와 포트를 조회합니다.
	BOOL			GetLocalIP(std::string& ipaddress,int &port);
	/// @brief 소켓에 바인딩된 로컬 포트를 조회합니다.
	USHORT			GetLocalPort(VOID);
	
	/// @brief 지정한 IPv4 주소와 포트에 TCP 연결을 시도합니다.
	BOOL			Connect(LPSTR address, USHORT port);
	/// @brief 지정한 원격 주소로 TCP 연결을 시도하고 해당 소켓 핸들을 반환합니다.
	SOCKET			Connect2(LPSTR address, USHORT port);

	/// @brief 현재 소켓 핸들 값을 반환합니다.
	SOCKET			GetSocket(VOID);
	/// @brief 소켓을 지정한 완료 포트에 연결하고 송신에 사용할 IOCP 참조를 보관합니다.
	BOOL RegisterIocp(GSNetwork::GSIocp::GSIocp& iocp, ULONG_PTR completionKey);

	// [FIX4] 2026-11-27 AcceptEx로 받은 소켓에 리슨 소켓의 속성을 상속시킨다 (SO_UPDATE_ACCEPT_CONTEXT).
	// 설정하지 않으면 getpeername / getsockname / shutdown / 일부 setsockopt가 실패한다.
	/// @brief AcceptEx가 끝난 소켓에 리스닝 소켓의 컨텍스트를 적용합니다.
	BOOL UpdateAcceptContext(SOCKET listenSocket);
	/// @brief 리스너 잠금 아래 해당 리스너의 Accept 컨텍스트를 적용합니다.
	BOOL UpdateAcceptContext(GSSocket& listener);
	
	/// @brief 문자열 또는 숫자 작업 정보를 포함해 소켓 예외를 기록하는 필터입니다.
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, char *Info);
	/// @brief 문자열 또는 숫자 작업 정보를 포함해 소켓 예외를 기록하는 필터입니다.
	long			ExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo, DWORD Info);

	/// @brief 내부 수신 버퍼의 시작 주소를 반환합니다.
	BYTE *GetBuffer();
};

}	}