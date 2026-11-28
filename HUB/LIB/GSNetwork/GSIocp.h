#pragma once

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>

struct _OVERLAPPED_EX;

class GSClient;

namespace GSNetwork	{	namespace GSIocp	{

class GSIocp
{
public:
	GSIocp(void);
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
	bool IsWorkerThread() const;
	virtual VOID		OnRead(int client_id, DWORD dataLength)	= 0;
	virtual VOID		OnWrote(int client_id, DWORD dataLength)	= 0;
	virtual VOID		OnConnected(int client_id)				= 0;
	virtual VOID		OnDisconnected(int client_id, bool isForce = false)			= 0;
	virtual VOID		OnDisconnected2(int client_id,int type) = 0;

public:
	BOOL				Initialize(VOID);
	BOOL				Termination(VOID);
	size_t GetPendingWriteCount();
	
	BOOL				RegIocpHandler(SOCKET socket, ULONG_PTR completionKey);
	BOOL SubmitWrite(SOCKET socket, std::unique_ptr<_OVERLAPPED_EX> operation, DWORD length, bool addressed);

	VOID				WorkerThread(VOID);

private:
	// [FIX2] 워커에서 예외가 났을 때 멈춘 연결 정리
	VOID				CleanupAfterWorkerException(int ObjectId);

public:
	
	LONG				ExceptionFilter( LPEXCEPTION_POINTERS pExceptionInfo, LPCSTR szPosition );


	// [FIX3] 2026-11-27 사용 금지: 핸들 값만 받아 소켓 락 없이 closesocket 하므로 핸들 재사용 문제가 생긴다.
	// 대신 GSClient::Close() 또는 GSSocket::CloseSocket()을 쓴다. (기존 코드 호환을 위해 함수는 남겨 둠)
	[[deprecated("Use GSClient::Close() or GSSocket::CloseSocket() instead (closes under the socket lock).")]]
	VOID				Close(SOCKET client);
};

}	}