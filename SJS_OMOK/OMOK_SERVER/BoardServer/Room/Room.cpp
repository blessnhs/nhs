#include "StdAfx.h"
#include "./Room.h"
#include "../Server/GSBoard.h"

DWORD Room::FindPlayer(PLAYER_PTR Player)
{
	for each(auto iter in m_PlayerMap)
	{
		PlayerPtr pPlayer = iter.second;
		if (pPlayer == NULL)
			continue;

		if (iter.second == Player)
		{
			return iter.first;
		}
	}

	return USHRT_MAX;
}

Room::Room(void) 
{
	InitializeCriticalSection(&m_PublicLock);

	for (int i = 0; i < COLS; i++)
		for (int j = 0; j < ROWS; j++)
			m_Board[i][j] = None;

	m_State = State::Prepare;

	m_TurnIndex = 0;
}

void Room::ClearBoard()
{
	for (int i = 0; i < COLS; i++)
		for (int j = 0; j < ROWS; j++)
			m_Board[i][j] = None;

	m_TurnIndex = 0;
}

Room::~Room(void)
{
	DeleteCriticalSection(&m_PublicLock);
}

bool Room::GetBoard(int x, int y, eTeam &_team)
{
	if (x < 0 || y < 0)
		return false;

	if (x >= ROWS || y >= COLS)
		return false;

	_team = m_Board[y][x];

	return true;

}

void Room::UpdateBoard(int x, int y, eTeam team)
{
	if (x < 0 || y < 0)
		return;

	if (x >= ROWS || y >= COLS)
		return;

	m_Board[y][x] = team;
}

void Room::SetRoomState(State state)
{
	m_State = state;
}

bool  Room::CheckGameResult(int _x, int _y, eTeam _stone)   // ���� ���� ���� ��ǥ�� ����(1or2) ������ ����
{
	int x, y;
	int count;

	//����
	x = _x;
	y = _y;
	count = 0;
	// x���� ���ҽ��� stone�� �ٸ������� ����
	while (x > 0 && m_Board[_y][ x - 1] == _stone)
		x--;

	// x���� �������� stone�� �ٸ������� ����
	while (x < COLS && m_Board[_y][ x++] == _stone)
		count++;

	if (count == 5)  // x���� 5�� ���������� ��� GameWin�Լ� ȣ��.
		return true;

	//����
	x = _x;
	y = _y;
	count = 0;
	// �̹����� y���� ��ȭ���� ���ο� ���������� ������ üũ�Ѵ�.
	while (y > 0 && m_Board[y - 1][ _x] == _stone)
		y--;
	while (y < ROWS && m_Board[y++][ _x] == _stone)
		count++;

	if (count == 5)
		return true;

	//�밢����
	x = _x;
	y = _y;
	count = 0;
	// x, y ��� ��ȭ���� ���� ���� ������� ������ üũ.
	while (x > 0 && y > 0 && m_Board[y - 1][ x - 1] == _stone)
	{
		y--;
		x--;
	}

	while (x < COLS && y < ROWS && m_Board[y++][ x++] == _stone)
		count++;

	if (count == 5)
		return true;

	//�밢����
	x = _x;
	y = _y;
	count = 0;
	// x, y ��� ��ȭ���� ���� ���� ������� ������ üũ.
	while ((x + 1) < COLS && y > 0 && m_Board[y - 1][ x + 1] == _stone)
	{
		x++;
		y--;
	}
	while (x >= 0 && y < ROWS && m_Board[y++][x--] == _stone)
		count++;

	if (count == 5)
		return true;


	return false;
}
bool Room::RemovePlayer(PLAYER_PTR Player)
{
	for each(auto iter in m_PlayerMap)
	{
		PlayerPtr pPlayer = iter.second;
		if (pPlayer == NULL)
			continue;

		if (iter.second == Player)
		{
			m_PlayerMap[iter.first] = NULL;
			return TRUE;
		}
	
	}

	return FALSE;
}

string Room::LevelConverter(int level)
{
	int o = 19 - level;
	string str, i;

	if (o <= 0)
	{
		int l = std::abs(o) + 1;
		if (l >= 9)
			l = 9;


		i = std::to_string(l);

		str.append(i);
		str.append("��");
		return str;
	}

	i = std::to_string(o);

	str.append(i);
	str.append("��");
	return str;

}

bool Room::InsertPlayer(PLAYER_PTR Player)
{
	for(int i=0;i<m_Stock.MAX_PLAYER;i++)
	{
		if(m_PlayerMap.find(i) == m_PlayerMap.end())
		{
			m_PlayerMap[i] = Player;
			Player->m_Char[0].SetTeam( WHITE );
			Player->m_Char[0].SetRoom(GetId());
			return true;
		}
	}

	return false;
}

bool Room::CreateGameRule(BYTE Type)
{
	switch(Type)
	{
	case 0:
		return TRUE;
	}

	return FALSE;
	
}


void Room::SendNewUserInfo(PLAYER_PTR Player)
{
	if(Player == NULL)	return ;

	for each(auto iter in m_PlayerMap)
	{
		PlayerPtr pPlayer = iter.second;
		if (pPlayer == NULL)
			continue;

		if (iter.second != NULL)
		{
			GSCLIENT_PTR pPair = SERVER.GetClient(iter.second->GetPair());
			if (pPair != NULL)
			{
				NEW_USER_IN_ROOM_NTY nty;

				RoomUserInfo *userinfo = nty.mutable_var_room_user();

				userinfo->set_var_index(Player->GetId());
				userinfo->set_var_name(Player->m_Account.GetName());
				userinfo->set_picture_uri(Player->m_Account.GetPicture_url());
				SEND_PROTO_BUFFER(nty, pPair)
			}
		}

	}


}

bool Room::IsAllComplete()
{
	for each(auto iter in m_PlayerMap)
	{
		PlayerPtr pPlayer = iter.second;
		if (pPlayer == NULL)
			continue;
		
		if (iter.second != NULL)
		{
			if (iter.second->m_Char[0].GetAllComplete() == false)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

void Room::SetState(State state)
{
	m_State = state;
}
State Room::GetState()
{
	return m_State;
}

WORD Room::GetCurrPlayer()
{
	int count = 0;
	for each (auto & player in m_PlayerMap)
	{
		if (player.second != NULL)
			count++;
	}
	return count;
}

PLAYER_PTR Room::GetOtherPlayer(DWORD INDEX)
{
	for each (auto & player in m_PlayerMap)
	{
		if (player.second == NULL)
			continue;

		if (player.second->GetId() == INDEX)
			continue;

		return player.second;

	}
	return NULL;
}

eTeam Room::GetTeamPlayer(DWORD INDEX)
{
	//��Ī�� 0���� ������ �� 1���� ȭ��Ʈ�� ó���ϵ�����
	int pos = 0;
	for each (auto & player in m_PlayerMap)
	{
		if (player.second == NULL)
			continue;

		if (player.second->GetId() != INDEX)
		{
			pos++;
			continue;
		}

		if(pos == 0)
			return eTeam::BLACK;
		else
			return eTeam::WHITE;

		pos++;

	}
	return eTeam::None;
}

void Room::RecoardResult(PLAYER_PTR Winner, PLAYER_PTR Loser)
{
	GAME_RESULT_NTY result_nty;
	result_nty.set_var_code(Success);


	//���߿� �ʿ信 ���� ���ν����� �ϳ��� �����ؾ� �Ұ� ����
	//�й� ����
	{

		Loser->m_Char[0].UpdateScore(-POINT);

		result_nty.set_var_index_1(Loser->GetId()); 
		result_nty.set_var_level_1(Loser->m_Char[0].GetLevel()); 
		result_nty.set_var_level_point_1(Loser->m_Char[0].GetScore().GetScorePoint());

		boost::shared_ptr<Board::MSG_PLAYER_QUERY<RequestPlayerScore>>		PLAYER_MSG = ALLOCATOR.Create<Board::MSG_PLAYER_QUERY<RequestPlayerScore>>();
		{
			PLAYER_MSG->pRequst.Index = Loser->GetId();
			PLAYER_MSG->pRequst.Win = 0;	PLAYER_MSG->pRequst.Lose = 1;	PLAYER_MSG->pRequst.Draw = 0; PLAYER_MSG->pRequst.Level = Loser->m_Char[0].GetLevel(); 
			PLAYER_MSG->pRequst.Score = Loser->m_Char[0].GetScore().GetScorePoint();

		}
		PLAYER_MSG->Type = MSG_TYPE_DB_1;
		PLAYER_MSG->SubType = ONQUERY;
		MAINPROC.RegisterCommand(PLAYER_MSG);
	}

	//�¼� ����
	{
		result_nty.set_var_index(Winner->GetId());
		result_nty.set_var_name(Winner->m_Account.GetName());

		eTeam color = GetTeamPlayer(Winner->GetId());
		result_nty.set_var_color(color);


		Winner->m_Char[0].UpdateScore(POINT);

		{

			result_nty.set_var_index_2(Winner->GetId());
			result_nty.set_var_level_2(Winner->m_Char[0].GetLevel());
			result_nty.set_var_level_point_2(Winner->m_Char[0].GetScore().GetScorePoint());

			boost::shared_ptr<Board::MSG_PLAYER_QUERY<RequestPlayerScore>>		PLAYER_MSG = ALLOCATOR.Create<Board::MSG_PLAYER_QUERY<RequestPlayerScore>>();
			{
				PLAYER_MSG->pRequst.Index = Winner->GetId();
				PLAYER_MSG->pRequst.Win = 1;	PLAYER_MSG->pRequst.Lose = 0;	PLAYER_MSG->pRequst.Draw = 0; PLAYER_MSG->pRequst.Level = Winner->m_Char[0].GetLevel();
				PLAYER_MSG->pRequst.Score = Winner->m_Char[0].GetScore().GetScorePoint();

			}
			PLAYER_MSG->Type = MSG_TYPE_DB_1;
			PLAYER_MSG->SubType = ONQUERY;
			MAINPROC.RegisterCommand(PLAYER_MSG);
		}
	}

	SendToAll(result_nty);
}

void Room::SendToAll(WORD MainId, BYTE * Data, WORD Length)
{
	for each (auto iter in m_PlayerMap)
	{
		PlayerPtr pPlayer = iter.second;
		if (pPlayer == NULL)
			continue;

		GSCLIENT_PTR pSession = SERVER.GetClient(iter.second->GetPair());

		if (pSession)
			pSession->GetTCPSocket()->WritePacket(MainId, 0, Data, Length);
	}
}

void Room::SET_X_Y_COLOR(byte X, byte Y, byte COLOR, int &FLAG)
{
	FLAG = X & 0xFF;

	int temp = Y & 0xFF;
	temp = temp << 8;
	FLAG = FLAG | temp;

	int _color = FLAG >> 16;
	_color = _color | COLOR;
	_color = _color << 16;
	FLAG = FLAG | _color;
}

bool Room::Get_X_Y_COLOR(byte &X, byte &Y, byte &Color, int FLAG)
{
	X = (byte)(FLAG & 0xFF);
	Y = (byte)(FLAG >> 8);
	Color = (byte)((FLAG >> 16) & 0x01);

	return true;
}

DWORD Room::GetTurnPlayerId()
{
	int turn = m_TurnIndex % m_PlayerMap.size();

	if (m_PlayerMap.find(turn) == m_PlayerMap.end())
	{
		return -1;
	}

	auto player = m_PlayerMap[turn];
	if (player != NULL)
	{
		return player->GetId();
	}

	return -1;
}

void  Room::IncTurnPlayerId()
{
	m_TurnIndex++;
}
