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
	virtual VOID		OnRead(int client_id, DWORD dataLength)	= 0;
	virtual VOID		OnWrote(int client_id, DWORD dataLength)	= 0;
	virtual VOID		OnConnected(int client_id)				= 0;
	virtual VOID		OnDisconnected(int client_id, bool isForce = false)			= 0;
	virtual VOID		OnDisconnected2(int client_id,int type) = 0;

public:
	BOOL				Initialize(VOID);
	BOOL				Termination(VOID);
	
	BOOL				RegIocpHandler(SOCKET socket, ULONG_PTR completionKey);
	BOOL SubmitWrite(SOCKET socket, std::unique_ptr<_OVERLAPPED_EX> operation, DWORD length, bool addressed);

	VOID				WorkerThread(VOID);
	
	LONG				ExceptionFilter( LPEXCEPTION_POINTERS pExceptionInfo, LPCSTR szPosition );


	VOID				Close(SOCKET client);
};

}	}