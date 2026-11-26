
// [LOCKFIX] 2026-11-26
//  m_RoomMap / m_RoomMapForLoop(concurrent_unordered_map)의 값(shared_ptr)은
//  boost::atomic_load / atomic_store / atomic_compare_exchange로만 읽고 쓴다.
//  (로직 스레드와 DB 스레드가 동시에 접근. 컨테이너는 값 대입을 보호하지 않음)

// 루프용 맵에서 빈(NULL) 슬롯을 원자적으로 차지 (두 스레드가 같은 빈 슬롯을 동시에 가져가지 못함)
template<class MapT, class PtrT>
inline bool RoomContainer_PutLoopSlot(MapT& LoopMap, PtrT& pObj)
{
	for (auto& kv : LoopMap)
	{
		PtrT expected;
		if (boost::atomic_compare_exchange(&kv.second, &expected, pObj))
			return true;
	}

	boost::atomic_store(&LoopMap[pObj->GetId()], pObj);
	return false;
}

// 루프용 맵에서 같은 id의 슬롯을 원자적으로 비움
template<class MapT, class PtrT>
inline bool RoomContainer_ClearLoopSlot(MapT& LoopMap, PtrT& pObj)
{
	for (auto& kv : LoopMap)
	{
		PtrT cur = boost::atomic_load(&kv.second);
		if (cur != NULL && cur->GetId() == pObj->GetId())
		{
			if (boost::atomic_compare_exchange(&kv.second, &cur, PtrT()))
				return true;
		}
	}
	return false;
}

template<template<class T> class CreationPolicy> bool RoomContainer<CreationPolicy>::Add(ROOM_PTR &pObj, PLAYER_PTR player)
{
	auto iter = m_RoomMap.find(pObj->GetId());

	if(iter == m_RoomMap.end())
	{
		// [LOCKFIX] find 후 삽입은 원자적이지 않으므로 빈 값일 때만 CAS로 넣는다
		ROOM_PTR expected;
		if (boost::atomic_compare_exchange(&m_RoomMap[pObj->GetId()], &expected, pObj) == false)
			return false;	// 다른 스레드가 같은 id로 먼저 추가

		//룸 매니저는 이제 매니저 없애고 채널에 귀속
	/*	if (player != NULL)
			CHANNELMGR.RoomEnterChannel(player->GetChannel(), pObj);
		else
			printf("cant enter channel %d\n", player->GetChannel());*/

		return RoomContainer_PutLoopSlot(m_RoomMapForLoop, pObj);
	}

	return false;
}

template<template<class T> class CreationPolicy> bool RoomContainer<CreationPolicy>::Del(ROOM_PTR &pObj, PLAYER_PTR player)
{
	auto iter = m_RoomMap.find(pObj->GetId());

	if(iter == m_RoomMap.end())
	{
		return false;
	}

	boost::atomic_store(&iter->second, ROOM_PTR());

	//if (player != NULL)
	//	CHANNELMGR.RoomLeaveChannel(player);
	//else
	//	printf("cant leave channel %d\n", player->GetChannel());

	return RoomContainer_ClearLoopSlot(m_RoomMapForLoop, pObj);
}


template<template<class T> class CreationPolicy> bool RoomContainer<CreationPolicy>::Add2(ROOM_PTR& pObj)
{
	auto iter = m_RoomMap.find(pObj->GetId());

	if (iter == m_RoomMap.end())
	{
		ROOM_PTR expected;
		if (boost::atomic_compare_exchange(&m_RoomMap[pObj->GetId()], &expected, pObj) == false)
			return false;

		return RoomContainer_PutLoopSlot(m_RoomMapForLoop, pObj);
	}

	return false;
}

template<template<class T> class CreationPolicy> bool RoomContainer<CreationPolicy>::Del2(ROOM_PTR& pObj)
{
	auto iter = m_RoomMap.find(pObj->GetId());

	if (iter == m_RoomMap.end())
	{
		return false;
	}

	boost::atomic_store(&iter->second, ROOM_PTR());

	return RoomContainer_ClearLoopSlot(m_RoomMapForLoop, pObj);
}


template<template<class T> class CreationPolicy> ROOM_PTR RoomContainer<CreationPolicy>::Search(INT64 Id)
{
	auto iter = m_RoomMap.find((DWORD)Id);

	if(iter == m_RoomMap.end())
	{
		return ROOM_PTR();
	}
	return boost::atomic_load(&iter->second);
}

template<template<class T> class CreationPolicy> ROOM_PTR RoomContainer<CreationPolicy>::SearchByGameType(WORD Type)
{
	for (auto& kv : m_RoomMap)
	{
		ROOM_PTR room = boost::atomic_load(&kv.second);
		if (room == NULL)
			continue;

		if (room->m_Stock.GameType == Type)
		{
			if (room->GetCurrPlayer() < room->m_Stock.MAX_PLAYER)
			{
				return room;
			}
		}
	}

	return ROOM_PTR();
}


template<template<class T> class CreationPolicy> ROOM_PTR RoomContainer<CreationPolicy>::Create()
{
	// [LOCKFIX] 이전: m_PublicLock으로 감쌌지만 팩토리(GSCreator)는 자체 락이 있고 id는 atomic이라 락이 필요 없었다.
	//           SetId(intAtomic) 후 fetch_add(1) 두 단계를 fetch_add 한 번으로 합침 (반환값은 증가 전 값 = 이전과 같은 id)
	static atomic<int> intAtomic = 1;

	ROOM_PTR PTR =  CreationPolicy<ROOM>().Create();

	PTR->SetId(intAtomic.fetch_add(1));

	return PTR;
}

template<template<class T> class CreationPolicy> ROOM_PTR RoomContainer<CreationPolicy>::Create(int room_id,string name)
{
	// [LOCKFIX] 락 불필요 (지역 객체만 다룸)
	ROOM_PTR PTR = CreationPolicy<ROOM>().Create();

	PTR->SetId(room_id);
	PTR->m_Stock.Name = name;

	return PTR;
}

template<template<class T> class CreationPolicy> VOID RoomContainer<CreationPolicy>::GetRoomList(google::protobuf::RepeatedPtrField<RoomInfo2>*List)
{
	const int max_count = 10;
	int currcount = 0;

	for (auto& kv : m_RoomMapForLoop)
	{
		ROOM_PTR room = boost::atomic_load(&kv.second);
		if (room != NULL)
		{
			if (currcount > max_count)
				break;

			RoomInfo2* info = List->Add();
			info->mutable_var_name()->assign(room->m_Stock.Name);
			info->set_var_id(room->GetId());
			info->set_var_current_count(room->GetCurrPlayer());
			info->set_var_max_count(-1);

			currcount++;
		}
	}
}

template<template<class T> class CreationPolicy> void RoomContainer<CreationPolicy>::Print()
{
	for (auto& kv : m_RoomMapForLoop)
	{
		ROOM_PTR room = boost::atomic_load(&kv.second);
		if (room == NULL)		// [LOCKFIX] 삭제된 방(NULL) 역참조 방지
			continue;

		printf("id :%d  name : %s count : %d\n", room->GetId(), room->m_Stock.Name.c_str(), room->GetCurrPlayer());
	}
}

template<template<class T> class CreationPolicy> int RoomContainer<CreationPolicy>::Count()
{
	int count = 0;
	for (auto& kv : m_RoomMapForLoop)
	{
		if (boost::atomic_load(&kv.second) != NULL)
			count++;
	}

	return count;
}

template<template<class T> class CreationPolicy>  const concurrency::concurrent_unordered_map<DWORD, ROOM_PTR>& RoomContainer<CreationPolicy>::GetRoomMap()
{
	return m_RoomMap;
}
template<template<class T> class CreationPolicy> void RoomContainer<CreationPolicy>::LeaveRoomPlayer(PLAYER_PTR pPlayer,int room_number, bool isdelete = false)
{
	LEAVE_ROOM_RES res;

	ROOM_PTR RoomPtr = Search(room_number);
	if (RoomPtr != NULL)
	{
		if (RoomPtr->FindPlayer(pPlayer) == TRUE)
		{

			res.set_var_index(pPlayer->GetId());
			res.set_var_code(Success);
			res.set_var_room_number(RoomPtr->GetId());
			res.mutable_var_name()->assign(pPlayer->m_Account.GetName());

			RoomPtr->SendToAll(res);

			RoomPtr->RemovePlayer(pPlayer);

			if (RoomPtr->GetCurrPlayer() == 0 && isdelete == true)
				Del(RoomPtr, pPlayer);
		}

	}
}
