#include "StdAfx.h"
#include <chrono>
#include "GSClient.h"
#include "GSMainProcess.h"
#include "MSG_PLAYER_CLOSE.h"
#include "MSG_PLAYER_ACCEPT.h"
#include "MSG_PLAYER_PACKET.h"

#include "GSAllocator.h"
#include "GSServer.h"
#include "IUdpProcess.h"

//namespace GSNetwork	{	namespace GSClient	{

atomic<int>		DebugCount = 0;

namespace
{
	std::atomic<unsigned long long> ClientsCreated{0};
	std::atomic<unsigned long long> ClientsDestroyed{0};
	std::atomic<unsigned long long> ClientsRemoved{0};
	std::atomic<unsigned long long> IdsRetiredWithPendingIo{0};
	std::atomic<int> PeakLiveClients{0};
	std::atomic<int> RemovedClientsStillLive{0};
	std::mutex DetachedClientsLock;
	GSClient* FirstDetachedClient = nullptr;
	GSClient* LastDetachedClient = nullptr;
	unsigned long long MaxDetachedMs = 0;
	std::atomic<unsigned long long> DuplicateCloseIgnored{0};
}

GSClientLifetimeStats GSClient::GetLifetimeStats()
{
	return {
		ClientsCreated.load(std::memory_order_relaxed),
		ClientsDestroyed.load(std::memory_order_relaxed),
		ClientsRemoved.load(std::memory_order_relaxed),
		IdsRetiredWithPendingIo.load(std::memory_order_relaxed),
		PeakLiveClients.load(std::memory_order_relaxed),
		RemovedClientsStillLive.load(std::memory_order_relaxed)
	};
}

void GSClient::MarkRemovedFromMap()
{
	std::lock_guard<std::mutex> lock(DetachedClientsLock);
	m_DetachedAt = GetTickCount64();
	m_DetachedId = GetId();
	m_DetachedPrevious = LastDetachedClient;
	if (LastDetachedClient)
		LastDetachedClient->m_DetachedNext = this;
	else
		FirstDetachedClient = this;
	LastDetachedClient = this;
	ClientsRemoved.fetch_add(1, std::memory_order_relaxed);
	RemovedClientsStillLive.fetch_add(1, std::memory_order_relaxed);
}

void GSClient::LogDetachedClients()
{
	struct Sample { DWORD Id; ULONGLONG AgeMs; };
	Sample samples[4]{};
	unsigned sampleCount = 0;
	unsigned overdue = 0;
	unsigned long long longestCompletedMs = 0;
	{
		std::lock_guard<std::mutex> lock(DetachedClientsLock);
		const auto now = GetTickCount64();
		for (auto* client = FirstDetachedClient; client; client = client->m_DetachedNext)
		{
			const auto age = now - client->m_DetachedAt;
			if (age < 30000)
				break;
			++overdue;
			if (sampleCount < _countof(samples))
				samples[sampleCount++] = {client->m_DetachedId, age};
		}
		longestCompletedMs = MaxDetachedMs;
	}
	SYSLOG().Write("[GSClient/cleanup] DetachedOver30s=%u MaxCompletedDetachMs=%llu DuplicateCloseIgnored=%llu\n",
		overdue, longestCompletedMs, DuplicateCloseIgnored.load(std::memory_order_relaxed));
	for (unsigned i = 0; i < sampleCount; ++i)
		SYSLOG().Write("[GSClient/cleanup-delay] ClientId=%lu DetachedMs=%llu\n", samples[i].Id, samples[i].AgeMs);
}

extern atomic<int>		DisConnectCount;

GSClient::GSClient(void)
{
	m_bConnected			= FALSE;
	m_Process				= NULL;
	m_AliveTime				= 0;
	m_WillBeTerminated		= FALSE;
	m_DBStamp				= 0;
	m_DBStampTime			= 0;
	m_PairPlayerId			= 0;
	m_GSServer				= NULL;
	m_DeleteTime			= 0;
	m_Socket = 0;
	m_TryCloseCount = 0;

	ClientsCreated.fetch_add(1, std::memory_order_relaxed);
	const int live = DebugCount.fetch_add(1, std::memory_order_relaxed) + 1;
	int peak = PeakLiveClients.load(std::memory_order_relaxed);
	while (live > peak &&
		!PeakLiveClients.compare_exchange_weak(peak, live, std::memory_order_relaxed))
	{
	}
}

boost::shared_ptr<GSPacketTCP>	GSClient::GetTCPSocket()
{
	return m_TCPSocket;
}

boost::shared_ptr<GSPacketUDP>	GSClient::GetUDPSocket()
{
	return m_UDPSocket;
}

void GSClient::SetAcceptListener(boost::shared_ptr<GSPacketTCP> listener)
{
	m_AcceptListener = std::move(listener);
}

const boost::shared_ptr<GSPacketTCP>& GSClient::GetAcceptListener() const
{
	return m_AcceptListener;
}

bool GSClient::ClaimAcceptReplacement()
{
	return m_AcceptListener && !m_AcceptReplacementClaimed.exchange(true);
}

GSClient::~GSClient(void)
{
	bool canRecycleId = true;
	if (m_TCPSocket)
	{
		const int accepts = m_TCPSocket->m_OLP_REMAIN_COUNT_ACC.load();
		const int reads = m_TCPSocket->m_OLP_REMAIN_COUNT_REC.load();
		const int writes = m_TCPSocket->m_OLP_REMAIN_COUNT_SND.load();
		canRecycleId = accepts == 0 && reads == 0 && writes == 0;
		if (!canRecycleId)
		{
			SYSLOG().Write("Client %lu retired with completions pending (accept=%d, read=%d, write=%d)\n",
				GetId(), accepts, reads, writes);
		}
	}


	// Completions carry only the client ID. Retire IDs with outstanding I/O.
	auto* server = static_cast<GSServer::GSServer*>(m_GSServer);
	if (server)
	{
		if (canRecycleId)
			server->GetClientMgr().InsertRecycleId(GetId());
		else
			IdsRetiredWithPendingIo.fetch_add(1, std::memory_order_relaxed);
	}

	if (m_TCPSocket != NULL)
		m_TCPSocket->Termination();

	if (m_UDPSocket != NULL)
		m_UDPSocket->Termination();

	{
		std::lock_guard<std::mutex> lock(DetachedClientsLock);
		if (m_DetachedPrevious || FirstDetachedClient == this)
		{
			if (m_DetachedPrevious)
				m_DetachedPrevious->m_DetachedNext = m_DetachedNext;
			else
				FirstDetachedClient = m_DetachedNext;
			if (m_DetachedNext)
				m_DetachedNext->m_DetachedPrevious = m_DetachedPrevious;
			else
				LastDetachedClient = m_DetachedPrevious;
			const auto age = GetTickCount64() - m_DetachedAt;
			if (age > MaxDetachedMs)
				MaxDetachedMs = age;
			RemovedClientsStillLive.fetch_sub(1, std::memory_order_relaxed);
		}
	}

	ClientsDestroyed.fetch_add(1, std::memory_order_relaxed);
	DebugCount.fetch_sub(1, std::memory_order_relaxed);
}

BOOL  GSClient::Create(BYTE Type,ClientType type)
{
	m_ClientType = type;
	m_CreateType = Type;

	if(Type == TCP)
	{
		boost::shared_ptr<GSPacketTCP> TCP(new GSPacketTCP);
		m_TCPSocket = TCP;
		m_TCPSocket->m_ClientId = GetId();
	}
	else if(Type == UDP)
	{
		boost::shared_ptr<GSPacketUDP> UDP(new GSPacketUDP);
		m_UDPSocket = UDP;
		m_UDPSocket->m_ClientId = GetId();
	}
	else
		return FALSE;

	return TRUE;
}

INT64 GSClient::GetPair()
{
	return m_PairPlayerId;
}

void GSClient::SetPair(INT64 Player)
{
	m_PairPlayerId = Player;
}

SOCKET GSClient::GetSocket(VOID)
{
	
	if(m_CreateType == TCP)
	{
		return m_TCPSocket->GetSocket();
	}
	else if(m_CreateType == UDP)
	{
		return m_UDPSocket->GetSocket();
	}

	return 0;
}

bool GSClient::GetCompleteJob(const std::wstring str)
{
	CThreadSync Sync(this);

	std::map<std::wstring,bool>::iterator iter = m_CheckCompleteMap.find(str);
	if(iter == m_CheckCompleteMap.end())
	{
		return false;
	}

	return m_CheckCompleteMap[str];
}

void GSClient::SetCompleteJob(const std::wstring str,bool Value)
{
	CThreadSync Sync(this);

	m_CheckCompleteMap[str] = Value;
}

BOOL GSClient::InitializeReadForIocp(VOID)
{
	if(m_CreateType == TCP)
	{
		return m_TCPSocket->InitializeReadForIocp();
	}
	else if(m_CreateType == UDP)
	{
		return m_UDPSocket->InitializeReadFromForIocp();
	}

	return FALSE;
}

BOOL GSClient::WriteComplete(VOID)
{
	if(m_CreateType == TCP)
	{
		return m_TCPSocket->WriteComplete();
	}
	else if(m_CreateType == UDP)
	{
		return m_UDPSocket->WriteComplete();
	}

	return FALSE;
}

VOID GSClient::SetConnected(BOOL bConnected)
{
	m_bConnected = bConnected;
}

BOOL GSClient::GetConnected(VOID)
{
	return m_bConnected;
}

boost::shared_ptr<IProcess<int>> GSClient::GetProcess()
{
	// [LOCKFIX] SetProcess(접속 처리 스레드)와 GetProcess(로직 스레드)가 동시에 일어날 수 있음
	return boost::atomic_load(&m_Process);
}

void GSClient::SetProcess(boost::shared_ptr<IProcess<int>> Process)
{
	CThreadSync Sync(this);

	if(Process != NULL)
		Process->pOwnerId = GetId();	// [LOCKFIX] 공개(store)하기 전에 초기화

	boost::atomic_store(&m_Process, Process);
}

VOID GSClient::OnResponse(LPVOID Data)
{
}

void GSClient::SetAliveTime(DWORD Time)
{
	m_AliveTime.store(Time, std::memory_order_relaxed);
}

DWORD GSClient::GetAliveTime()
{
	return m_AliveTime.load(std::memory_order_relaxed);
}

void GSClient::IncDBStamp()
{
	++m_DBStamp;

	m_DBStampTime = GetTickCount();
}

void GSClient::DecDBStamp()
{
	--m_DBStamp;
}

DWORD GSClient::GetDBStamp()
{
	return m_DBStamp;
}

void GSClient::SetWillBeTerminated(bool _terminate)
{
	m_WillBeTerminated = _terminate;
}

BOOL GSClient::GetWillBeTerminated()
{
	return m_WillBeTerminated;
}

VOID GSClient::TakeMsg(boost::shared_ptr<GSClient> client)
{
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	MSG_PLAYER_PACKET_PTR	pPlayerPacket = ALLOCATOR.Create<MSG_PLAYER_PACKET>();

	pPlayerPacket->pHandler= this;
	pPlayerPacket->Type	= GetMyTP();
	pPlayerPacket->SubType = ONPACKET;
	pPlayerPacket->pClient = client;

	MAINPROC.RegisterCommand(pPlayerPacket);
}

void GSClient::OnEvt(IMessagePtr Arg)
{

}
VOID GSClient::Close()
{
	// [FIX3] 2026-11-27 이전: closesocket(GetTCPSocket()->GetSocket())
	//  -> 소켓 락 없이 핸들을 닫고 m_Socket도 그대로 남겨서, 닫힌 핸들 값이 새 소켓에 재사용되면
	//     다른 스레드가 엉뚱한 연결로 송수신할 수 있었다. UDP 클라이언트에서 호출하면 NULL 역참조.
	// 변경: 소켓 락 아래에서 닫고 m_Socket을 NULL로 만든다. 걸려 있던 수신은 실패로 완료되어
	//       평소처럼 IO_READ 실패 경로로 종료 처리가 한 번 실행된다.
	boost::shared_ptr<GSPacketTCP> tcp = GetTCPSocket();
	if (tcp != NULL)
		tcp->CloseSocket();
}


ClientType	GSClient::GetClientType()
{
	return m_ClientType;
}

BYTE GSClient::GetCreateType()
{
	return m_CreateType;
}

VOID GSClient::ProcPacket(boost::shared_ptr<GSClient> pClient)
{
	boost::shared_ptr<XDATA> pBuffer;

	SetAliveTime(GetTickCount());

	if (GetCreateType() == TCP)
	{
		while (GetTCPSocket()->m_PacketList.try_pop(pBuffer) == TRUE)
		{
			if (pBuffer == NULL) return;

			auto handler = GetProcess();
			if (!handler)
			{
				SYSLOG().Write("set process is null");
				return;
			}

			handler->Process(pBuffer, pClient);
		}
	}
	else if (GetCreateType() == UDP)
	{
		while (GetUDPSocket()->m_PacketList.try_pop(pBuffer) == TRUE)
		{
			if (pBuffer == NULL) return;

			auto handler = GetProcess();
			if (!handler)
			{
				static std::atomic<unsigned long long> missingHandler{0};
				const auto total = missingHandler.fetch_add(1, std::memory_order_relaxed) + 1;
				if (total <= 4 || total % 1024 == 0)
					SYSLOG().Write("[UDP/no-handler] ClientId=%lu EchoPackets=%llu\n", GetId(), total);

				//echo
				GetUDPSocket()->WriteTo2(const_cast<char *>(pBuffer->LemoteAddress.c_str()), pBuffer->RemotePort, pBuffer->m_Buffer.GetBuffer(), pBuffer->Length);
				return;			
			}

			handler->Process2(pBuffer, pClient, pBuffer->LemoteAddress, pBuffer->RemotePort);
		}
	}

}

VOID GSClient::ProcDisconnect(boost::shared_ptr<GSClient> pClient,bool isForce)
{
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	//accpet를 안거치고 바로 disconnect되는 애들은 this->GetProcess() null이다 
	//그래서 그넘들은 종료 처리를 해저야하기 때문에 아래에서 리턴시키면 안된다.
	if(GetConnected() == FALSE && isForce == false && this->GetProcess() != NULL)
	{
		const auto duplicates = DuplicateCloseIgnored.fetch_add(1, std::memory_order_relaxed) + 1;
		if (duplicates <= 4 || duplicates % 1024 == 0)
			SYSLOG().Write("[GSClient/duplicate-close] ClientId=%lu Total=%llu\n", GetId(), duplicates);
		return ;
	}

	//접속 종료시 로직 쓰레드에 있는 잡은 아래 함수에서 disconnect에서 유저를 빼면 처리가 안될수 있다.
	//iocp worker thread와 로직 쓰레드와 처리가 다르기 때문이다.
	//남은 것을 처리하고 마지막에 종료 시키려면 pServer->Disconnect(pClient); 로직쓰레드로 던져야 한다.
	//
	//소켓을 close 시켜서 OnDisconnect가 떨어져도 player객체가 남아 있는 경우는 
	//소켓을 종료 체크하는 alive 체크 함수 쓰레드와 ProcDisconnect쓰레드가 다른 경우 
	//소켓 멤버를 clear()한 다음 유저 객체 초기화가되어 유저를 못찾았을 가능성이 크다.

	DisConnectCount.fetch_add(1);


	//컨텐츠 종료 처리
	
	//쉽게 말해서 iocp work -> logic -> db thread
    // db thread에서 실행중일때  iocp work에서 disconnect가 먼저 떨어지면  그때는 유저가 없기 때문에 그냥 종료 처리 그 후에 db login처리가 되어 user가 미아가 됨
	//고민하다가 그냥 logic tp로 던지고 유저 체크 함수를 만듬 (alive check usermgr)


	PROC_REG_CLOSE_JOB(pClient,pServer)


	//상태값 초기화
	SetConnected(FALSE);

	//pServer->GetClientMgr().BookDelClient(pClient->GetId());

	pServer->GetClientMgr().DelClient(pClient->GetId());


}


WORD GSClient::GetMyTP()
{
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	WORD TP = (GetId() % pServer->GetArgument().m_LogicThreadCnt);
	if(TP < 0)
	{
		SYSLOG().Write("TP is low %d\n",TP);
	}

	return TP;
}

WORD GSClient::GetMyDBTP(WORD StartIndex)
{
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	return StartIndex + (GetId() % pServer->GetArgument().m_DBThreadCnt);
}

void GSClient::OnSend(WORD MainId,WORD SubId,char *Data,WORD Length)
{

}

void GSClient::OnRecv(DWORD Length, boost::shared_ptr<GSClient> client)
{
	CThreadSync Sync(this);

	if (client->GetCreateType() == TCP)
	{
		GetTCPSocket()->MakePacket(Length);

		TakeMsg(client);
	}
	else if (client->GetCreateType() == UDP)
	{
		try
		{
			GetUDPSocket()->MakePacket(Length);
		}
		catch (...)
		{
			GetUDPSocket()->InitializeReadFromForIocp();
			throw;
		}

		// The parser has copied the datagram out of the receive buffer.
		GetUDPSocket()->InitializeReadFromForIocp();

		//udp의 경우 속도가 중요하고 connectionless이기 때문에 로직 스레드로 던지지 않고 그냥 처리해본다.
		//TakeMsg(client);

		const auto handlerStart = std::chrono::steady_clock::now();
		client->ProcPacket(client);
		const auto elapsed = std::chrono::steady_clock::now() - handlerStart;
		GetUDPSocket()->RecordHandlerTime(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
	}
}

void GSClient::OnDisconnect(boost::shared_ptr<GSClient> client, bool isForce)
{
	CThreadSync Sync(this);

/*	if (m_TCPSocket->m_Accept_OLP->Overlapped.Internal != 0)
	{
		DWORD Code = GetLastError();
		if(Code == ERROR_NETNAME_DELETED)
		{
			if(m_PairPlayerId != ULONG_MAX)
			{
				SYSLOG().Write("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ERROR_NETNAME_DELETED\n");
			}

			SYSLOG().Write("Code %d ERROR_NETNAME_DELETED\n",Code);

			ProcDisconnect(client,isForce);

			return ;
		}

		SYSLOG().Write("Overlapped.Internal != 0\n");
	}*/

	//소켓 접속 여유 풀이 없을때 만약 100명이면 100명다 붙어 있을때 클라에서 소켓 연결 요청하면
	//connect에는 콜이 안떨어진다 그 상태에서 접속 종료하면 disconnected만 떨어짐 Connected exception ...2

	if(GetProcess() == NULL)
		SYSLOG().Write("!!!!!!!!!!!!!OnDisconnect get process is nulll\n");

	//GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	//아래 함수를 로직 쓰레드로 던지게 되면 동기화 문제가 발생하여 
	//바로 콜하는 것으로 대체
	//PROC_REG_CLOSE_JOB(this,this,pServer)

	ProcDisconnect(client, isForce);
	
}

void GSClient::OnConnect(boost::shared_ptr<GSClient> pClient)
{
	//Accept가 떨어졌다.
	CThreadSync Sync(this);

	//SYSLOG().Write("Accept Success Socket %d %d %d\n",GetSocket(),GetId(),GetMyTP());


	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	SetAliveTime(GetTickCount());

	pServer->Accept(pClient);
	pClient->SetConnected(true);

	//아래 함수를 로직 쓰레드로 던지게 되면 동기화 문제가 발생하여 
	//바로 콜하는 것으로 대체


	string ip;
	int port = 0;
	pClient->GetTCPSocket()->GetLocalIP(ip, port);

#ifdef _DEBUG
//	printf("connected ip %s port %d\n",ip.c_str(),port);
#endif

	/*
	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	MSG_PLAYER_ACCEPT_PTR pPlayerAccept = ALLOCATOR.Create<MSG_PLAYER_ACCEPT>(); //(new MSG_PLAYER_ACCEPT);// 
	pPlayerAccept->pClient = this;
	pPlayerAccept->pHandler= this;
	pPlayerAccept->Type	= GetMyTP();
	pPlayerAccept->SubType = ONACCEPT;

	MAINPROC.RegisterCommand(pPlayerAccept);
	*/
}

VOID GSClient::Clear()
{
	CThreadSync Sync(this);

	m_AliveTime = 0;

	m_PairPlayerId  = ULONG_MAX;	
	m_AliveTime = 0;
	m_WillBeTerminated   = FALSE;
	m_DBStamp = 0;
	m_bConnected = FALSE;
	m_DBStampTime = 0;

	m_CheckAbuseMap.clear();
	m_CheckCompleteMap.clear();

	m_DeleteTime = 0;
}

DWORD GSClient::GetDBStampTime()
{
	return m_DBStampTime;
}


//}	}