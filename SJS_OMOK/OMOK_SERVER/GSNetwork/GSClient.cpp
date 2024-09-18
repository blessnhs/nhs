#include "StdAfx.h"
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

	DebugCount.fetch_add(1);
}

boost::shared_ptr<GSPacketTCP>	GSClient::GetTCPSocket()
{
	return m_TCPSocket;
}

boost::shared_ptr<GSPacketUDP>	GSClient::GetUDPSocket()
{
	return m_UDPSocket;
}

GSClient::~GSClient(void)
{
	int count = GetTCPSocket()->m_OLP_REMAIN_COUNT_ACC + GetTCPSocket()->m_OLP_REMAIN_COUNT_REC + GetTCPSocket()->m_OLP_REMAIN_COUNT_SND;
	if (count > 0)
	{
		SYSLOG().Write("GetId() %d alert remain overlap event count acc %d recv %d send %d\n",GetId(), GetTCPSocket()->m_OLP_REMAIN_COUNT_ACC.fetch_add(0), GetTCPSocket()->m_OLP_REMAIN_COUNT_REC.fetch_add(0), GetTCPSocket()->m_OLP_REMAIN_COUNT_SND.fetch_add(0));
	}

	DebugCount.fetch_sub(1);

	GSServer::GSServer* pServer = (GSServer::GSServer*)m_GSServer;

	pServer->GetClientMgr().InsertRecycleId(GetId());
}

BOOL  GSClient::Create(BYTE Type)
{
	CThreadSync Sync;
	
	if(Type == TCP)
	{
		boost::shared_ptr<GSPacketTCP> TCP(new GSPacketTCP);
		m_TCPSocket = TCP;
		m_TCPSocket->m_ClientId = GetId();

		m_CreateType = Type;
	}
	else if(Type == UDP)
	{
		boost::shared_ptr<GSPacketUDP> UDP(new GSPacketUDP);
		m_UDPSocket = UDP;
		m_UDPSocket->m_ClientId = GetId();

		m_CreateType = Type;
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

/*BOOL GSClient::Recycle(SOCKET hListenSocket)
{
	CThreadSync Sync;

	if(m_CreateType == TCP)
	{
		m_TCPSocket->Termination();

		return m_TCPSocket->Initialize() && m_TCPSocket->Accept(hListenSocket);
	}
	else if(m_CreateType == UDP)
	{
		m_UDPSocket->Termination();

		return m_UDPSocket->Initialize() && m_UDPSocket->Accept(hListenSocket);
	}

	return FALSE;
}*/

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
	CThreadSync Sync;

	std::map<std::wstring,bool>::iterator iter = m_CheckCompleteMap.find(str);
	if(iter == m_CheckCompleteMap.end())
	{
		return false;
	}

	return m_CheckCompleteMap[str];
}

void GSClient::SetCompleteJob(const std::wstring str,bool Value)
{
	CThreadSync Sync;

	m_CheckCompleteMap[str] = Value;
}

BOOL GSClient::InitializeReadForIocp(VOID)
{
	CThreadSync Sync;

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
	CThreadSync Sync;

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
	CThreadSync Sync;

	m_bConnected = bConnected;
}

BOOL GSClient::GetConnected(VOID)
{
	CThreadSync Sync;

	return m_bConnected;
}

boost::shared_ptr<IProcess<int>> GSClient::GetProcess()
{
	return m_Process;
}

void GSClient::SetProcess(boost::shared_ptr<IProcess<int>> Process)
{
	CThreadSync Sync;

	m_Process = Process;

	if(m_Process != NULL)
		m_Process->pOwnerId = GetId();
}

VOID GSClient::OnResponse(LPVOID Data)
{
}

void GSClient::SetAliveTime(DWORD Time)
{
	m_AliveTime = Time;
}

DWORD GSClient::GetAliveTime()
{
	return m_AliveTime;
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
	closesocket(GetSocket());
}

BYTE GSClient::GetCreateType()
{
	return m_CreateType;
}

VOID GSClient::ProcPacket(boost::shared_ptr<GSClient> pClient)
{
	CThreadSync Sync;

	boost::shared_ptr<XDATA> pBuffer;

	SetAliveTime(GetTickCount());

	if (GetCreateType() == TCP)
	{
		int size = (int)GetTCPSocket()->m_PacketList.unsafe_size();

		while (GetTCPSocket()->m_PacketList.try_pop(pBuffer) == TRUE)
		{
			size = (int)GetTCPSocket()->m_PacketList.unsafe_size();

			if (pBuffer == NULL) return;

			if (GetProcess() == NULL)
			{
				SYSLOG().Write("set process is null");
				return;
			}

			GetProcess()->Process(pBuffer->m_Buffer.GetBuffer(), pBuffer->Length, pBuffer->MainId, pBuffer->SubId, pClient);
		}
	}
	else if (GetCreateType() == UDP)
	{
		int size = (int)GetUDPSocket()->m_PacketList.unsafe_size();

		while (GetUDPSocket()->m_PacketList.try_pop(pBuffer) == TRUE)
		{
			size = (int)GetUDPSocket()->m_PacketList.unsafe_size();

			if (pBuffer == NULL) return;

			if (GetProcess() == NULL)
			{
				SYSLOG().Write("set process is null\n");

				//echo
				GetUDPSocket()->WriteTo2(const_cast<char *>(pBuffer->LemoteAddress.c_str()), pBuffer->RemotePort, pBuffer->m_Buffer.GetBuffer(), pBuffer->Length);
				return;			
			}

			GetProcess()->Process2(pBuffer->m_Buffer.GetBuffer(), pBuffer->Length, pBuffer->MainId, pBuffer->SubId, pClient, pBuffer->LemoteAddress, pBuffer->RemotePort);
		}
	}

}

VOID GSClient::ProcDisconnect(boost::shared_ptr<GSClient> pClient,bool isForce)
{
	CThreadSync Sync;

	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	//accpet�� �Ȱ�ġ�� �ٷ� disconnect�Ǵ� �ֵ��� this->GetProcess() null�̴� 
	//�׷��� �׳ѵ��� ���� ó���� �������ϱ� ������ �Ʒ����� ���Ͻ�Ű�� �ȵȴ�.
	if(GetConnected() == FALSE && isForce == false && this->GetProcess() != NULL)
	{
		SYSLOG().Write("Already Disconnected socket handle %lu getid() %lu \n",(unsigned long)GetSocket(),GetId());
		return ;
	}

	//���� ����� ���� �����忡 �ִ� ���� �Ʒ� �Լ����� disconnect���� ������ ���� ó���� �ȵɼ� �ִ�.
	//iocp worker thread�� ���� ������� ó���� �ٸ��� �����̴�.
	//���� ���� ó���ϰ� �������� ���� ��Ű���� pServer->Disconnect(pClient); ����������� ������ �Ѵ�.
	//
	//������ close ���Ѽ� OnDisconnect�� �������� player��ü�� ���� �ִ� ���� 
	//������ ���� üũ�ϴ� alive üũ �Լ� ������� ProcDisconnect�����尡 �ٸ� ��� 
	//���� ����� clear()�� ���� ���� ��ü �ʱ�ȭ�� ȭ��Ǿ� ������ ��ã���� ���ɼ��� ũ��.



	//������ ���� ó��

	//���� �����尡 �ƴ϶� �ٷ� db tp�� ������.
	//google auth���� �����̰� �ɷ��� �Ʒ� ���� ó�� �Լ��� ���� ����Ǹ�( concurrent ���� �� playermgr ���� ,roommgr ����)
	//concurrent �Լ����� �����͸� ������� ����.
	//�׷��� closejob ����ǰ� login �Լ��� ����Ǹ� ������ ������ ������ ���� �ȴ�.
	//�׷��� �Ϻ����� �ʴ� logic -> dbthread 2�ܰ��̱� ����
	//����ϴٰ� �׳� logic tp�� ������ ���� üũ �Լ��� ���� (alive check usermgr)

	PROC_REG_CLOSE_JOB(pClient,pServer)


	//���°� �ʱ�ȭ
	SetConnected(FALSE);

	pServer->GetClientMgr().BookDelClient(pClient->GetId());

	pServer->GetClientMgr().NewClient();

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
	CThreadSync Sync;

	WORD  MainProtocol = 0,SubProtocol = 0;
	DWORD dwPacketLength = 0;

	if (client->GetCreateType() == TCP)
	{
		GetTCPSocket()->MakePacket(Length);

		TakeMsg(client);
	}
	else if (client->GetCreateType() == UDP)
	{
		GetUDPSocket()->MakePacket(Length, MainProtocol, SubProtocol, dwPacketLength);

		//udp�� ��� �ӵ��� �߿��ϰ� connectionless�̱� ������ ���� ������� ������ �ʰ� �׳� ó���غ���.
		//TakeMsg(client);

		client->ProcPacket(client);
	}
}

void GSClient::OnDisconnect(boost::shared_ptr<GSClient> client, bool isForce)
{
	CThreadSync Sync;

	if(m_TCPSocket->m_Accept_OLP->Overlapped.Internal != 0)
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
	}

	//���� ���� ���� Ǯ�� ������ ���� 100���̸� 100��� �پ� ������ Ŭ�󿡼� ���� ���� ��û�ϸ�
	//connect���� ���� �ȶ������� �� ���¿��� ���� �����ϸ� disconnected�� ������ Connected exception ...2

	if(GetProcess() == NULL)
		SYSLOG().Write("!!!!!!!!!!!!!OnDisconnect get process is nulll\n");

	//GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	//�Ʒ� �Լ��� ���� ������� ������ �Ǹ� ����ȭ ������ �߻��Ͽ� 
	//�ٷ� ���ϴ� ������ ��ü
	//PROC_REG_CLOSE_JOB(this,this,pServer)

	ProcDisconnect(client, isForce);
	
}

void GSClient::OnConnect(boost::shared_ptr<GSClient> pClient)
{
	//Accept�� ��������.
	CThreadSync Sync;

	//SYSLOG().Write("Accept Success Socket %d %d %d\n",GetSocket(),GetId(),GetMyTP());


	GSServer::GSServer *pServer = (GSServer::GSServer *)m_GSServer;

	SetAliveTime(GetTickCount());

	pServer->Accept(pClient);
	pClient->SetConnected(true);

	//�Ʒ� �Լ��� ���� ������� ������ �Ǹ� ����ȭ ������ �߻��Ͽ� 
	//�ٷ� ���ϴ� ������ ��ü


	string ip;
	pClient->GetTCPSocket()->GetLocalIP(ip);

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
	CThreadSync Sync;

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