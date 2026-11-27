#include "StdAfx.h"
#include "./Room.h"
#include "../Server/GSHub.h"


int  Room::FindPlayerPos(PLAYER_PTR Player)
{
	int pos = 0;
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);	// [LOCKFIX]
		if (pPlayer == NULL)
			continue;

		if (pPlayer->GetId() == Player->GetId())
		{
			return pos;
		}

		pos++;
	}

	return 0;
}

bool Room::FindPlayer(PLAYER_PTR Player)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);	// [LOCKFIX]
		if (pPlayer == NULL)
			continue;

		if (pPlayer->GetId() == Player->GetId())
		{
			return true;
		}
	}

	return false;
}

Room::Room(void) 
{
	InitializeCriticalSection(&m_PublicLock);
}

Room::~Room(void)
{
	m_MessageList.clear();

	DeleteCriticalSection(&m_PublicLock);
}

bool Room::RemovePlayer(PLAYER_PTR Player)
{
	boost::atomic_store(&m_PlayerMap[Player->GetId()], PlayerPtr());	// [LOCKFIX]
	Player->DelRoom(GetId());
	return FALSE;
}

bool Room::InsertPlayer(PLAYER_PTR Player)
{
	boost::atomic_store(&m_PlayerMap[Player->GetId()], Player);	// [LOCKFIX]
	Player->SetRoom(GetId());
	return true;
}

void Room::SendNewUserInfo(PLAYER_PTR Player,int RoomNumber)
{
	if(Player == NULL)	return ;

	// [LOCKFIX] 보내는 내용이 모든 대상에게 같으므로 한 번만 만들어서 SendToAll로 전송 (이전: 대상마다 생성/직렬화)
	NEW_USER_IN_ROOM_NTY nty;

	RoomUserInfo *userinfo = nty.mutable_var_room_user();

	userinfo->set_var_index(Player->GetId());
	userinfo->set_var_name(Player->m_Account.GetName());
	userinfo->set_picture_uri(Player->m_Account.GetPicture_url());
	userinfo->set_var_room_number(RoomNumber);

	// 이전 코드와 같이 GetPair()로 세션을 찾는다
	size_t BufSize = nty.ByteSizeLong();
	std::vector<BYTE> Buffer(BufSize + 1);
	nty.SerializeToArray(&Buffer[0], (int)BufSize);

	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);
		if (pPlayer == NULL)
			continue;

		GSCLIENT_PTR pPair = SERVER.GetClient(pPlayer->GetPair());
		if (pPair != NULL && pPair->GetConnected() == TRUE)
		{
			pPair->GetTCPSocket()->WritePacket(nty.id(), 0, &Buffer[0], (DWORD)BufSize, pPlayer->GetFrontSid());
		}
	}
}

WORD Room::GetCurrPlayer()
{
	int count = 0;
	for (auto& kv : m_PlayerMap)
	{
		if (boost::atomic_load(&kv.second) != NULL)	// [LOCKFIX]
			count++;
	}
	return count;
}

void Room::SendToAll(WORD MainId, BYTE * Data, WORD Length)
{
	for (auto& kv : m_PlayerMap)
	{
		PlayerPtr pPlayer = boost::atomic_load(&kv.second);	// [LOCKFIX]
		if (pPlayer == NULL)
			continue;

		GSCLIENT_PTR pSession = SERVER.GetClient(pPlayer->GetPair());

		if (pSession)
			pSession->GetTCPSocket()->WritePacket(MainId, 0, Data, Length, pPlayer->GetFrontSid());
	}
}


VOID Room::GetMessageList(google::protobuf::RepeatedPtrField<RoomMessage>* List)
{
	// [LOCKFIX] m_MessageList(std::list)는 AddRoomMessage와 동시에 접근될 수 있음 (Front별 로직 스레드, DB 스레드)
	EnterCriticalSection(&m_PublicLock);

	for each (auto& msg in m_MessageList)
	{
		RoomMessage* info = List->Add();
		info->mutable_var_name()->assign(msg.var_name());

		info->set_var_message(msg.var_message());
		info->set_var_message_int(msg.var_message_int());
		info->set_var_time(msg.var_time());
	}

	LeaveCriticalSection(&m_PublicLock);
}

void  Room::AddRoomMessage(RoomMessage msg)
{
	EnterCriticalSection(&m_PublicLock);	// [LOCKFIX]

	if (m_MessageList.size() > 100)
		m_MessageList.pop_front();

	m_MessageList.push_back(msg);

	LeaveCriticalSection(&m_PublicLock);
}
