#pragma once

#include "Structure.pb.h"

using namespace std;
typedef GSNetwork::GSObject::GSObject GSObject;

struct RoomStock
{
	string Name;
	string Passwd;
	WORD  MAX_PLAYER;
	WORD  GameType;

};

#define COLS  20         // 20
#define	ROWS  20         // 20

class Room : public GSObject 
{
public:
	Room(void);
	~Room(void);

	RoomStock	m_Stock;

	bool InsertPlayer(PLAYER_PTR Player);

	void SendNewUserInfo(PLAYER_PTR Player, int RoomNumber);

	bool FindPlayer(PLAYER_PTR Player);

	int  FindPlayerPos(PLAYER_PTR Player);

	bool RemovePlayer(PLAYER_PTR Player);

	WORD GetCurrPlayer();

	// [LOCKFIX] 2026-11-26
	//  이전: 플레이어마다 SEND_PROTO_BUFFER -> protobuf 직렬화 + new/delete + (760바이트 초과 시) gzip 압축을 N번 반복
	//  변경: 한 번만 직렬화해서 같은 버퍼를 모든 대상에게 보낸다.
	//  m_PlayerMap 값은 다른 스레드가 바꿀 수 있으므로 atomic_load로 복사해서 사용한다.
	template<class T>
	void SendToAll(T& snd)
	{
		SendToAllImpl(snd, false, 0);
	}

	template<class T>
	void SendToAll(T& snd,int PlayerId)
	{
		SendToAllImpl(snd, true, PlayerId);
	}

	template<class T>
	void SendToAllImpl(T& snd, bool SkipPlayer, int PlayerId)
	{
		size_t BufSize = snd.ByteSizeLong();
		std::vector<BYTE> Buffer(BufSize + 1);		// +1: 크기 0인 메시지도 NULL이 아닌 포인터를 넘기기 위함
		snd.SerializeToArray(&Buffer[0], (int)BufSize);

		for (auto& kv : m_PlayerMap)
		{
			PlayerPtr pPlayer = boost::atomic_load(&kv.second);
			if (pPlayer == NULL)
				continue;

			if (SkipPlayer && pPlayer->GetId() == PlayerId)
				continue;

			GSCLIENT_PTR pSession = SERVER.GetClient(pPlayer->GetFront());

			if (pSession && pSession->GetConnected() == TRUE)
				pSession->GetTCPSocket()->WritePacket(snd.id(), 0, &Buffer[0], (DWORD)BufSize, pPlayer->GetFrontSid());
		}
	}

	VOID GetMessageList(google::protobuf::RepeatedPtrField<RoomMessage>* List);


	void SendToAll(WORD MainId, BYTE* Data, WORD Length);
	
	concurrency::concurrent_unordered_map<DWORD, PLAYER_PTR>		m_PlayerMap;

	void AddRoomMessage(RoomMessage msg);

private:


	list<RoomMessage> m_MessageList;

	// [LOCKFIX] 이전에는 선언만 하고 쓰지 않았음. 이제 m_MessageList(std::list) 보호에 사용한다.
	CRITICAL_SECTION												m_PublicLock;

};

typedef boost::shared_ptr<Room> RoomPtr;



#define ROOM		Room 
#define ROOM_PTR	RoomPtr 
