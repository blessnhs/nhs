#pragma once

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>

struct _OVERLAPPED_EX;

class GSClient;

namespace GSNetwork	{	namespace GSIocp	{

/// @brief Windows 완료 포트와 워커를 관리하며 I/O 완료를 서버 콜백으로 전달합니다.
class GSIocp
{
public:
	/// @brief 완료 포트와 워커 관리에 필요한 상태를 초기화합니다.
	GSIocp(void);
	/// @brief IOCP 기반 객체의 소멸자입니다.
	virtual ~GSIocp(void);

private:
	HANDLE				m_Handle;
	DWORD				m_WTCnt;

	std::vector<HANDLE>	m_WTVector;

	HANDLE				m_BeginEvtHandle;
	std::shared_timed_mutex m_SubmissionLock;
	bool m_AcceptingWrites = false;
	std::mutex m_PendingWritesLock;
	std::condition_variable m_WritesDrained;
	std::map<OVERLAPPED*, SOCKET> m_PendingWrites;

protected:
	/// @brief 현재 스레드가 이 완료 포트의 워커인지 확인합니다.
	bool IsWorkerThread() const;
	/// @brief 수신 완료 바이트 수를 파생 서버에 전달합니다.
	virtual VOID		OnRead(int client_id, DWORD dataLength)	= 0;
	/// @brief 송신 완료 바이트 수를 파생 서버에 전달합니다.
	virtual VOID		OnWrote(int client_id, DWORD dataLength)	= 0;
	/// @brief Accept 완료를 파생 서버에 전달합니다.
	virtual VOID		OnConnected(int client_id)				= 0;
	/// @brief 대상 클라이언트의 연결 종료 처리를 파생 서버에 요청합니다.
	virtual VOID		OnDisconnected(int client_id, bool isForce = false)			= 0;
	/// @brief 실패한 I/O나 TCP EOF의 작업 종류와 클라이언트 ID를 전달합니다.
	virtual VOID		OnDisconnected2(int client_id,int type) = 0;

public:
	/// @brief 완료 포트를 생성하고 완료 큐를 소비할 워커를 시작합니다.
	BOOL				Initialize(VOID);
	/// @brief 새 송신을 막고 미완료 송신을 취소·회수한 뒤 워커와 포트를 닫습니다. 워커에서 호출하면 실패합니다.
	BOOL				Termination(VOID);
	/// @brief 완료 포트가 추적 중인 송신 작업 수를 반환합니다.
	size_t GetPendingWriteCount();
	
	/// @brief 소켓을 이 완료 포트에 연결하고 완료 키를 등록합니다.
	BOOL				RegIocpHandler(SOCKET socket, ULONG_PTR completionKey);
	/// @brief 송신 컨텍스트 소유권을 받아 WSASend 또는 WSASendTo를 등록합니다. 완료 패킷이 생성되지 않는 제출 실패는 호출 중에 정리합니다.
	BOOL SubmitWrite(SOCKET socket, std::unique_ptr<_OVERLAPPED_EX> operation, DWORD length, bool addressed);

	/// @brief 완료 포트에서 I/O 결과를 받아 컨텍스트를 회수하고 작업별 콜백을 호출합니다.
	VOID				WorkerThread(VOID);

private:
	// [FIX2] 워커에서 예외가 났을 때 멈춘 연결 정리
	/// @brief 워커 처리 중 예외가 난 클라이언트의 종료를 시도합니다.
	VOID				CleanupAfterWorkerException(int ObjectId);

public:
	
	/// @brief 워커의 구조적 예외를 진단하는 필터입니다.
	LONG				ExceptionFilter( LPEXCEPTION_POINTERS pExceptionInfo, LPCSTR szPosition );


	// [FIX3] 2026-11-27 사용 금지: 핸들 값만 받아 소켓 락 없이 closesocket 하므로 핸들 재사용 문제가 생긴다.
	// 대신 GSClient::Close() 또는 GSSocket::CloseSocket()을 쓴다. (기존 코드 호환을 위해 함수는 남겨 둠)
	/// @brief 원시 소켓 핸들을 닫는 레거시 API입니다. 현재 호출부는 소켓 객체의 CloseSocket을 사용합니다.
	[[deprecated("Use GSClient::Close() or GSSocket::CloseSocket() instead (closes under the socket lock).")]]
	VOID				Close(SOCKET client);
};

}	}