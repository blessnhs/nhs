#include "stdafx.h"

#include "GSHub.h"
#include "HubProcess.h"
#include "IUdpProcess.h"
#include "../PLAYER/PlayerContainer.h"
#include "../Room/RoomContainer.h"
#include "../DB/DBProxy/DBProcessContainer.h"

class RequestDeleteAllConcurrentUser;

GSHub::GSHub(void)
{
}


GSHub::~GSHub(void)
{
}

BOOL GSHub::Disconnect(GSCLIENT_PTR pSession)
{


	//해당 프론트가 떨어졌기 때문에 그 프론트에 유저는 날린다.
	//어차피 그 프론트가 죽었기 때문에 따로 kick 패킷도 필요 없다.
	PLAYERMGR.DelPlayerByFrontId(pSession->GetId());

	//if (pPlayer != NULL)
	//{
	//	
	//	for each (auto room in pPlayer->GetRoom())
	//	{
	//		ROOMMGR.LeaveRoomPlayer(pPlayer, room);
	//	}

	//	
	//	pPlayer->SetPair(ULONG_MAX);
	//	PLAYERMGR.Del(pPlayer);
	//}

	////로그아웃 쿼리를 날린다.
	//boost::shared_ptr<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>		PLAYER_MSG = ALLOCATOR.Create<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>();
	//PLAYER_MSG->pSession = pSession;

	//PLAYER_MSG->Request.m_args = std::tuple<INT64>(pSession->GetPair());

	//PLAYER_MSG->Type = pSession->GetMyDBTP();
	//PLAYER_MSG->SubType = ONQUERY;
	//MAINPROC.RegisterCommand(PLAYER_MSG);

	printf("disconnect front session \n");



	return TRUE;
}

VOID GSHub::Accept(GSCLIENT_PTR object)
{
	object->SetHandler<HubProcess>();
}

BOOL GSHub::DeleteAllConcurrentUser()
{
	//로그아웃 쿼리를 날린다.

	boost::shared_ptr<Hub::MSG_PLAYER_QUERY<Hub::RequestDeleteAllConcurrentUser>>		PLAYER_MSG = ALLOCATOR.Create<Hub::MSG_PLAYER_QUERY<Hub::RequestDeleteAllConcurrentUser>>();
	PLAYER_MSG->Type = MSG_TYPE_DB_1;
	PLAYER_MSG->SubType = ONQUERY;
	MAINPROC.RegisterCommand(PLAYER_MSG);

	return TRUE;
}



BOOL GSHub::Initialize()
{
	INI.Load();

	SYSTEM_INFO SystemInfo; 
	GetSystemInfo(&SystemInfo);
	WORD dwThreadNum = 2*SystemInfo.dwNumberOfProcessors;

	GSArgument _arg;

	_arg.m_BindPort  = m_Ini.Port;
	_arg.m_DBThreadCnt = dwThreadNum;
	_arg.m_LogicThreadCnt = dwThreadNum;
	_arg.m_MaxClient = m_Ini.MaxUser;
	if (!_arg.LoadTcpBindEndpoints("./Hub.ini"))
	{
		SYSLOG().Write("[TCP/startup] Invalid TCP_BIND configuration in Hub.ini\n");
		return FALSE;
	}
	_arg.m_Naggle = false;

	if (INI.AliveSeconds != -1)
		_arg.m_AliveTime = INI.AliveSeconds * 1000;
	else
		_arg.m_AliveTime = 0;

	_arg.m_UseCompress = true;

	DBPROCESSCONTAINER_CER.Initialize(_arg.m_DBThreadCnt);

	SetHandler<HubProcess>();

	DeleteAllConcurrentUser();

	ROOMMGR.CreateDBRoomList();

	const WORD primaryPort = _arg.m_TcpBindEndpoints.empty()
		? _arg.m_BindPort : _arg.m_TcpBindEndpoints.front().Port;
	if (!Create(_arg))
	{
		SYSLOG().Write("[HUB/startup] Network startup failed port=%u\n", static_cast<unsigned>(primaryPort));
		return FALSE;
	}
	SYSLOG().Write("[HUB/startup] Network listening primaryTcpPort=%u maxClients=%u\n",
		static_cast<unsigned>(primaryPort), static_cast<unsigned>(_arg.m_MaxClient));

	//default
	//
	


	
	return TRUE;
}
