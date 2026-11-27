#include "StdAfx.h"
#include "GSClientMgr.h"
#include "GSServer.h"
#include "ConsoleHelper.h"
#include <boost/make_shared.hpp>
#include "GSAllocator.h"

extern atomic<int>		DebugCount;
extern atomic<int>		ConnectCount;
extern atomic<int>		DisConnectCount;

atomic<int>				NewConnectount = 0;

namespace GSNetwork	{	namespace GSClientMgr	{

GSClientMgr::GSClientMgr(void)
{
}


GSClientMgr::~GSClientMgr(void)
{
}

VOID GSClientMgr::AllCloseSocket()
{
	for (auto& kv : m_Clients)
	{
		// [LOCKFIX] 값(shared_ptr)은 다른 스레드가 DelClient()에서 동시에 바꿀 수 있으므로 atomic_load로 복사해서 사용
		GSCLIENT_PTR client = boost::atomic_load(&kv.second);
		if (client == NULL)
		{
			continue;
		}

		if (client->GetConnected())
		{
				// [FIX3] 이전: pServer->Close(client->GetTCPSocket()->GetSocket()) -> 락 없이 핸들을 닫음
				client->Close();
		}
	}
}


VOID GSClientMgr::CheckAliveTime()
{
	GSServer::GSServer* pServer = (GSServer::GSServer*)m_GSServer;

	int debug_null_cnt = 0;
	int connection_cnt = 0;

	for (auto& kv : m_Clients)
	{
		// [LOCKFIX] atomic_load로 복사 (DelClient와의 shared_ptr 경쟁 방지)
		GSCLIENT_PTR client = boost::atomic_load(&kv.second);
		if (client == NULL)
		{
			debug_null_cnt++;
			continue;
		}

		if (client->GetConnected())
		{
			DWORD client_time = client->GetAliveTime();
			DWORD server_check_time = pServer->GetArgument().m_AliveTime;
			DWORD system_tick = GetTickCount();

			if (server_check_time != 0 && client_time != 0 && system_tick - client_time >= server_check_time)
			{
				if (client->GetClientType() != ClientType::SERVER_CLIENT)
				{
					client->m_TryCloseCount++;

					
					if(client->m_TryCloseCount > 1)
					{
						//이 코드는 사실상 들오면 안된다.
						//client->OnDisconnect(client.second, true);
					}
					else if(client->m_TryCloseCount == 1)
					{
						//여기서 서버에서 그냥 close하면 무한 무프 ondisconnect-> close 반복이라 이런식으로 iocp에서 감지하겠끔 해준다.
						// [LOCKFIX] 이전: 지역(스택) 배열 byte data[2]를 비동기 WSASend로 보냄
						//  -> 함수가 끝나면 버퍼가 사라지는데 커널은 아직 그 주소를 읽을 수 있음.
						//  -> 송신 큐에도 넣지 않아서 완료 시 WriteComplete()가 다른 송신 버퍼를 pop해서 해제함.
						// 변경: 힙에 복사하고 송신 큐에 등록하는 WriteRaw() 사용 (동작은 동일하게 1바이트 송신)
						byte data[1] = { 0 };
						client->GetTCPSocket()->WriteRaw(data, 1);
//						pServer->Close(client->GetTCPSocket()->GetSocket());
					}
				}
			}

			connection_cnt++;
		}
	}

	for (int i = 0; i < m_Remove_Queue.unsafe_size(); i++)
	{
		GSCLIENT_PTR client;
		if (m_Remove_Queue.try_pop(client) == true)
		{

			DWORD ClientTime = client->m_DeleteTime;
			DWORD SYSTime = GetTickCount();
			int count = client->GetTCPSocket()->m_OLP_REMAIN_COUNT_ACC + client->GetTCPSocket()->m_OLP_REMAIN_COUNT_REC + client->GetTCPSocket()->m_OLP_REMAIN_COUNT_SND;

			int Diff = SYSTime - ClientTime;

			//1분 넘으면 그냥 삭제한다.
			if ((ClientTime < SYSTime && count == 0) || (Diff > (1000 * 60 * 1)))
			{
				if (DelClient(client->GetId()) == FALSE)
				{
					SYSLOG().Write("DelClient is not 0 failed \n");
				}
			}
			else
				m_ReInsert_Queue.push(client);

			if ((ClientTime < SYSTime) && count != 0)
			{
				SYSLOG().Write("count is not 0 %d \n", count);
			}
		}
	}

	//다시 넣는다.  ㅠㅠ
	for (int i = 0; i < m_ReInsert_Queue.unsafe_size(); i++)
	{
		GSCLIENT_PTR client;
		if (m_ReInsert_Queue.try_pop(client) == true)
		{
			m_Remove_Queue.push(client);
		}
	}

//	SYSTEMTIME		sysTime;
//	::GetLocalTime(&sysTime);

//	SYSLOG().Write("[ %04d-%02d-%02d %02d:%02d:%02d ] Current User Count %d connectable count %d Debug Count %d\n",
//		sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, pServer->CurrentPlayerCount(), ConnectableSocketCount(), DebugCount.fetch_add(0));

//	SYSLOG().Write("\nre m_insert_queue queue %d\n", m_ReInsert_Queue.unsafe_size());
//	SYSLOG().Write("re m_remove_queue queue %d\n", m_Remove_Queue.unsafe_size());
//	SYSLOG().Write("\nconnect socket count %d GetActiveSocketCount %d ConnectableSocketCount %d\n", 
//	pServer->CurrentPlayerCount(), GetActiveSocketCount(), ConnectableSocketCount());

	//SYSLOG().Write("[UserCount : %d] [Debug Count : %d]\n",
	//	pServer->CurrentPlayerCount(), DebugCount.fetch_add(0));
	{

		char msg[256];
		
		sprintf_s(msg, 256, "[Conncted:%d] [Debug:%d] [RemoveQ:%zd] [accept :%d] [Toal New : %d][Total Close : %d] [debug_null %d]\n",
			connection_cnt, DebugCount.fetch_add(0), m_Remove_Queue.unsafe_size(), ConnectCount.fetch_add(0), NewConnectount.fetch_add(0), DisConnectCount.fetch_add(0), debug_null_cnt);
		//	pServer->TotalRecvBytes.fetch_add(0)/1024/1024,pServer->TotalSendBytes.fetch_add(0) / 1024 / 1024);

		ConsoleHelper::DebugConsoleString(0, msg);

	}
}

int GSClientMgr::GetActiveSocketCount()
{
	int count = 0;

	for (auto& kv : m_Clients)
	{
		GSCLIENT_PTR client = boost::atomic_load(&kv.second);	// [LOCKFIX]
		if (client == NULL)
		{
			continue;
		}

		if (client->GetConnected() == FALSE)
			continue;
		
		count++;
	}

	return count;
}

int GSClientMgr::IncClientId()
{
	static atomic<int> intAtomic = 0;

	int idx = intAtomic.fetch_add(1);         

	return idx;
}


GSCLIENT_PTR GSClientMgr::GetClient(int id)
{
	auto find = m_Clients.find(id);
	if (find == m_Clients.end())
		return NULL;

	// [LOCKFIX] 이전: find 후 operator[]로 한 번 더 조회 + 값 복사가 DelClient의 대입과 경쟁
	return boost::atomic_load(&find->second);

}

BOOL GSClientMgr::AddClient(GSCLIENT_PTR newclient)
{
	auto find = m_Clients.find(newclient->GetId());
	if (find != m_Clients.end())
	{
		if (boost::atomic_load(&find->second) != NULL)
			printf("alert add client id is exist\n");
	}

	newclient->m_GSServer = this->m_GSServer;	// [LOCKFIX] 다른 스레드에 보이기 전에 먼저 설정

	boost::atomic_store(&m_Clients[newclient->GetId()], newclient);	// [LOCKFIX]


	return TRUE;
}

BOOL GSClientMgr::BookDelClient(int id)
{
	GSCLIENT_PTR client = GetClient(id);	// [LOCKFIX] 임시 객체를 비const 참조로 받던 코드 정리

	if (client == NULL)
		return FALSE;

	//삭제 처리는 5초후에 일괄로처리한다. 
	client->m_DeleteTime = GetTickCount() + 5000;
	m_Remove_Queue.push(client);
	

	return TRUE;
}

BOOL GSClientMgr::DelClient(int id)
{
	// [LOCKFIX] 다른 스레드가 같은 값을 읽는 중일 수 있으므로 atomic_store
	// (concurrent_unordered_map은 삽입/조회만 스레드 안전하고 값 객체의 대입은 보호하지 않는다)
	boost::atomic_store(&m_Clients[id], GSCLIENT_PTR());

	return TRUE;
}

int GSClientMgr::PopRecycleId()
{
	//만번부터 
	if (m_Reycle_Id_Queue.unsafe_size() < 10000)
		return -1;

	int id;
	if (m_Reycle_Id_Queue.try_pop(id) == true)
		return id;

	return -1;
}

void GSClientMgr::InsertRecycleId(int _id)
{
	m_Reycle_Id_Queue.push(_id);
}


//모든 문제의 발단
//listen(backlog)수만큼 접속가능하고 거기서 감소 (iocp에서 떨어지는disconnect만큼 acceptex를 해주면딘다.)
//임의로 콜한 disconnect에서 잘못해줬다가는 time_wait close_wait같은 tcp 4hand shake상에서 close_wait이 남게된다.)
//접속 및 종료는 딱딱 맞춰줘야 한다.

BOOL GSClientMgr::NewClient(int NewClient)
{
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;
	SOCKET ListenSocket = pServer->GetTcpListen()->GetSocket();

	if (!ListenSocket)
	{
		SYSLOG().Write("NewClient !ListenSocket \n");
		return FALSE;
	}

	CThreadSync Sync(this);
			
	NewConnectount.fetch_add(NewClient);


	for (int i = 0; i < NewClient; i++)
	{
		GSCLIENT_PTR pClient = ALLOCATOR.Create<GSClient>();

		int newid = PopRecycleId();
		if (newid == -1)
			newid = IncClientId();

		pClient->SetId(newid);
		pClient->Create(TCP);
		pClient->m_GSServer = pServer;
		pClient->GetTCPSocket()->m_UseCompress = pServer->GetArgument().m_UseCompress;
		if (AddClient(pClient) == FALSE)
		{
			SYSLOG().Write("NewClient failed...1 \n");
		}


		if (!pClient->GetTCPSocket()->Initialize())
		{
			SYSLOG().Write("NewClient failed...2 \n");
			DelClient(pClient->GetId());
			End();
			return FALSE;
		}

		if (!pClient->GetTCPSocket()->Accept(ListenSocket))
		{
			SYSLOG().Write("NewClient failed...3 \n");
			DelClient(pClient->GetId());
			End();
			return FALSE;
		}
	}

	return TRUE;
}


GSCLIENT_PTR GSClientMgr::NewClient2()
{
	GSServer::GSServer* pServer = (GSServer::GSServer*)m_GSServer;
	SOCKET ListenSocket = pServer->GetTcpListen()->GetSocket();

	if (!ListenSocket)
	{
		SYSLOG().Write("NewClient !ListenSocket \n");
		return FALSE;
	}

	CThreadSync Sync(this);
	for (int i = 0; i < 1; i++)
	{
		GSCLIENT_PTR pClient = ALLOCATOR.Create<GSClient>();

		int newid = PopRecycleId();
		if (newid == -1)
			newid = IncClientId();

		pClient->SetId(newid);
		pClient->Create(TCP,ClientType::SERVER_CLIENT);
		pClient->m_GSServer = pServer;

		if (AddClient(pClient) == FALSE)
		{
			SYSLOG().Write("NewClient failed...1 \n");
		}

		return pClient;
	}

	return NULL;
}


BOOL GSClientMgr::Begin(SOCKET ListenSocket,WORD MaxClients,LPVOID pServer)
{
	if (!ListenSocket)
		return FALSE;

	DWORD MaxUser = m_MaxClients = MaxClients;


	GSServer::GSServer* Server = (GSServer::GSServer*)pServer;


	for (DWORD i=0;i<MaxUser;i++)
	{
		GSCLIENT_PTR pClient = ALLOCATOR.Create<GSClient>();
		pClient->SetId(IncClientId());
		pClient->Create(TCP);
		pClient->m_GSServer = pServer;
		pClient->GetTCPSocket()->m_UseCompress = Server->GetArgument().m_UseCompress;

		if (!pClient->GetTCPSocket()->Initialize())
		{
			End();
			return FALSE;
		}

		if (!pClient->GetTCPSocket()->Accept(ListenSocket))
		{
			End();
			return FALSE;
		}

		pClient->m_Socket = pClient->GetTCPSocket()->GetSocket();

		AddClient(pClient);
	}

	return TRUE;
}

VOID GSClientMgr::End()
{

}

}	}