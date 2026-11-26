#include "../Server/GSHub.h"
#include "../Room/RoomContainer.h"

class GSCLIENT;

// [LOCKFIX] 2026-11-26
//  m_PlayerMap / m_PlayerMapForLoop는 concurrency::concurrent_unordered_map이다.
//  이 컨테이너는 "삽입/조회/순회"만 스레드 안전하고, 이미 있는 값(shared_ptr)을 대입하는 것은 보호하지 않는다.
//  로직 스레드(Front별)와 DB 스레드가 동시에 Add/Del/Search를 하므로
//  값은 반드시 boost::atomic_load / atomic_store / atomic_compare_exchange로 읽고 쓴다.
//  (map[id] = NULL 과 다른 스레드의 값 복사가 겹치면 참조 카운트 손상, null 역참조가 날 수 있음)

template<template<class T> class CreationPolicy> PlayerContainer<CreationPolicy>::PlayerContainer()
{
}
template<template<class T> class CreationPolicy> PlayerContainer<CreationPolicy>::~PlayerContainer()
{
}

// 루프용 맵에서 빈(NULL) 슬롯을 원자적으로 차지한다. 두 스레드가 같은 빈 슬롯을 동시에 가져가지 못한다.
inline bool PlayerContainer_PutLoopSlot(
	concurrency::concurrent_unordered_map<INT64, PlayerPtr>& LoopMap, PlayerPtr& pObj)
{
	for (auto& kv : LoopMap)
	{
		PlayerPtr expected;
		if (boost::atomic_compare_exchange(&kv.second, &expected, pObj))
			return true;
	}

	boost::atomic_store(&LoopMap[pObj->GetId()], pObj);
	return true;
}

// 루프용 맵에서 pObj와 같은 id를 가진 슬롯을 원자적으로 비운다.
inline bool PlayerContainer_ClearLoopSlot(
	concurrency::concurrent_unordered_map<INT64, PlayerPtr>& LoopMap, PlayerPtr& pObj)
{
	for (auto& kv : LoopMap)
	{
		PlayerPtr cur = boost::atomic_load(&kv.second);
		if (cur != NULL && cur->GetId() == pObj->GetId())
		{
			if (boost::atomic_compare_exchange(&kv.second, &cur, PlayerPtr()))
				return true;
		}
	}
	return false;
}

template<template<class T> class CreationPolicy> bool PlayerContainer<CreationPolicy>::Add2(PlayerPtr& pObj)
{
	boost::atomic_store(&m_PlayerMap[pObj->GetId()], pObj);

	return PlayerContainer_PutLoopSlot(m_PlayerMapForLoop, pObj);
}

template<template<class T> class CreationPolicy> bool PlayerContainer<CreationPolicy>::Del2(PlayerPtr& pObj)
{
	auto iter = m_PlayerMap.find(pObj->GetId());

	if (iter == m_PlayerMap.end())
	{
		return false;
	}

	boost::atomic_store(&iter->second, PlayerPtr());

	///////////////////////////////////////////////////

	return PlayerContainer_ClearLoopSlot(m_PlayerMapForLoop, pObj);
}

template<template<class T> class CreationPolicy> bool PlayerContainer<CreationPolicy>::Add(PlayerPtr &pObj)
{
	DWORD id = GetIncId();
	pObj->SetId(id);

	boost::atomic_store(&m_PlayerMap[pObj->GetId()], pObj);

	return PlayerContainer_PutLoopSlot(m_PlayerMapForLoop, pObj);
}

template<template<class T> class CreationPolicy> bool PlayerContainer<CreationPolicy>::Del(PlayerPtr &pObj)
{
	auto iter = m_PlayerMap.find(pObj->GetId());

	if(iter == m_PlayerMap.end())
	{
		return false;
	}

	boost::atomic_store(&iter->second, PlayerPtr());

	///////////////////////////////////////////////////

	return PlayerContainer_ClearLoopSlot(m_PlayerMapForLoop, pObj);
}

template<template<class T> class CreationPolicy> void PlayerContainer<CreationPolicy>::DelPlayerByFrontId(DWORD FrontId)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr player = boost::atomic_load(&kv.second);
		if (player == NULL)
			continue;

		if (player->GetFront() == FrontId)
		{
			Del(player);
		}
	}
}

template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::Search(DWORD FrontId, DWORD FrontSid)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr player = boost::atomic_load(&kv.second);
		if (player == NULL)
			continue;

		if (player->GetFront() == FrontId && player->GetFrontSid() == FrontSid)
		{
			return player;
		}
	}

	return NULL;
}

template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::SearchByFrontSid(DWORD FrontSid)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr player = boost::atomic_load(&kv.second);
		if (player == NULL)
			continue;

		if (player->GetFrontSid() == FrontSid)
		{
			return player;
		}
	}

	return NULL;
}


template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::Search(INT64 Id)
{
	auto iter = m_PlayerMap.find(Id);

	if(iter == m_PlayerMap.end())
	{
		return PlayerPtr();
	}

	return boost::atomic_load(&iter->second);
}

template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::SearchBySocketId(DWORD Id)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if (pPlayer == NULL)
			continue;

		if (pPlayer->GetPair() == Id)
		{
			return pPlayer;
		}
	}

	return PlayerPtr();
}

template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::SearchByMachineId(string Id)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if (pPlayer == NULL)
			continue;

		if (pPlayer->GetMachineId() == Id)
		{
			return pPlayer;
		}
	}

	return PlayerPtr();
}


template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>:: Search(string flatformid)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if (pPlayer == NULL)
			continue;

		if (pPlayer->m_Account.GetFlatformId() == flatformid)
		{
			return pPlayer;
		}
	}
	
	return PlayerPtr();
}

template<template<class T> class CreationPolicy> int PlayerContainer<CreationPolicy>::Count()
{
	int count = 0;

	for (auto& kv : m_PlayerMapForLoop)
	{
		if (boost::atomic_load(&kv.second) != NULL)
		{
			count++;
		}
	}

	return count;
}
template<template<class T> class CreationPolicy> PlayerPtr PlayerContainer<CreationPolicy>::Create()
{
	return CreationPolicy<Player>().Create();
}

template<template<class T> class CreationPolicy> void PlayerContainer<CreationPolicy>::CheckUserList()
{
	for (auto iter = m_PlayerMapForLoop.begin(); iter != m_PlayerMapForLoop.end(); iter++)
	{
		PlayerPtr pPlayer = boost::atomic_load(&iter->second);
		if (pPlayer == NULL)
			continue;

		WORD SessionId = pPlayer->GetPair();

		DWORD CurrentTick = GetTickCount();

		//5분보다 크다
		if (CurrentTick > pPlayer->m_AliveTime + (1000 * 60 * 5))
		{
			BLOG("Player Client %lld AliveTime Check \n", pPlayer->GetId());

			for each (auto room in pPlayer->GetRoom())
			{
				ROOMMGR.LeaveRoomPlayer(pPlayer, room);
			}

			pPlayer->SetPair(ULONG_MAX);
			PLAYERMGR.Del(pPlayer);

			//로그아웃 쿼리를 날린다.

			boost::shared_ptr<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>		PLAYER_MSG = ALLOCATOR.Create<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>();
			PLAYER_MSG->Request.m_args = std::tuple<INT64>(pPlayer->GetId());
			PLAYER_MSG->Type = MSG_TYPE_DB_1;				//그냥 디폴트
			PLAYER_MSG->SubType = ONQUERY;
			MAINPROC.RegisterCommand(PLAYER_MSG);

			//로그아웃 날린다.
			GSCLIENT_PTR pSession = SERVER.GetClient(SessionId);
			if (pSession != NULL)
			{
				CLIENT_KICK kick;
				SEND_PROTO_BUFFER( kick, pSession, pPlayer->GetFrontSid())
			}

			continue;
		}

		//미아가 된 player들
		GSCLIENT_PTR pSession = SERVER.GetClient(SessionId);
		if (pSession == NULL)
		{
			BLOG("Player Client %lld Session mia\n",pPlayer->GetId());

			for each (auto room in pPlayer->GetRoom())
			{
				ROOMMGR.LeaveRoomPlayer(pPlayer,room);
			}

			pPlayer->SetPair(ULONG_MAX);
			PLAYERMGR.Del(pPlayer);

			//로그아웃 쿼리를 날린다.

			boost::shared_ptr<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>		PLAYER_MSG = ALLOCATOR.Create<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>();
			PLAYER_MSG->Request.m_args = std::tuple<INT64>(pPlayer->GetId());
			PLAYER_MSG->Type = MSG_TYPE_DB_1;				//그냥 디폴트
			PLAYER_MSG->SubType = ONQUERY;
			MAINPROC.RegisterCommand(PLAYER_MSG);
		}
		
	}
}


template<template<class T> class CreationPolicy> void PlayerContainer<CreationPolicy>::Print()
{
	int total = 0;
	for (auto& kv : m_PlayerMapForLoop)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if(pPlayer != NULL)
		{
			printf("%s\n",pPlayer->m_Account.GetName().c_str());
		}
		total++;
	}

	printf("\nasocket count %d\n",SERVER.GetClientMgr().GetActiveSocketCount());

	printf("Total Count %d\n", total);
}

template<template<class T> class CreationPolicy> template<class TYPE> void PlayerContainer<CreationPolicy>::BroadCast(TYPE MSG, INT64 INDEX)
{
	for (auto& kv : m_PlayerMapForLoop)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if (pPlayer != NULL && pPlayer->GetDBIndex() == INDEX)
		{
			auto pPair = SERVER.GetClient(pPlayer->GetPair());
			if (pPair != NULL)
			{
				SEND_PROTO_BUFFER(MSG, pPair, pPlayer->GetFrontSid())
			}
		}
	}
}


template<template<class T> class CreationPolicy> template<class TYPE> void PlayerContainer<CreationPolicy>::BroadCast(TYPE MSG)
{
	for (auto& kv : m_PlayerMapForLoop)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if(pPlayer != NULL)
		{
			GSCLIENT_PTR pPair = SERVER.GetClient(pPlayer->GetPair());
			if(pPair != NULL)
			{
				SEND_PROTO_BUFFER(MSG, pPair, pPlayer->GetFrontSid())
			}
		}
	}
}

template<template<class T> class CreationPolicy>  VOID PlayerContainer<CreationPolicy>::OnResponse(LPVOID Data)
{

}



template<template<class T> class CreationPolicy>  VOID PlayerContainer<CreationPolicy>::Disconnect(PlayerPtr pPlayer)
{
	if (pPlayer != NULL)
	{

		for each (auto room in pPlayer->GetRoom())
		{
			ROOMMGR.LeaveRoomPlayer(pPlayer, room);
		}


		pPlayer->SetPair(ULONG_MAX);
		PLAYERMGR.Del(pPlayer);
	}
}


template<template<class T> class CreationPolicy>  VOID PlayerContainer<CreationPolicy>::Disconnect(GSCLIENT_PTR pSession)
{
	PlayerPtr pPlayer = PLAYERMGR.Search(pSession->GetPair());
	if (pPlayer != NULL)
	{
		
		for each (auto room in pPlayer->GetRoom())
		{
			ROOMMGR.LeaveRoomPlayer(pPlayer, room);
		}

		
		pPlayer->SetPair(ULONG_MAX);
		PLAYERMGR.Del(pPlayer);
	}

	//로그아웃 쿼리를 날린다.
	boost::shared_ptr<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>		PLAYER_MSG = ALLOCATOR.Create<Hub::MSG_PLAYER_QUERY<Hub::RequestLogout>>();
	PLAYER_MSG->pSession = pSession;

	PLAYER_MSG->Request.m_args = std::tuple<INT64>(pSession->GetPair());

	PLAYER_MSG->Type = pSession->GetMyDBTP();
	PLAYER_MSG->SubType = ONQUERY;
	MAINPROC.RegisterCommand(PLAYER_MSG);
	
}