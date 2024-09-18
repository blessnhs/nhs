#include "StdAfx.h"
#include "./Character.h"

Character::Character()
{
	m_AllComplete = false;

	m_Level = 0;

}

Character::~Character() {}

std::string& Character::GetName()
{
	return m_Name;	
}

void Character::SetName(std::string& _Name)
{
	m_Name =  _Name;
}


void Character::SetAllComplete(bool _ready)
{
	m_AllComplete = _ready;
}

bool Character::GetAllComplete()
{
	return m_AllComplete;
}


void Character::SetTeam(bool _team)
{
	m_Team = _team;
}

byte Character::GetTeam()
{
	return m_Team;
}

void Character::SetReady(bool _ready)
{
	m_Ready = _ready;
}

bool Character::GetReady()
{
	return m_Ready;
}

VOID Character::DelRoom(DWORD _id)
{
	m_RoomNumbers.erase(_id);
}

VOID Character::SetRoom(DWORD _id)
{
	m_RoomNumbers.insert(_id);
}

std::set<DWORD> Character::GetRoom()
{
	return m_RoomNumbers;
}

Score& Character::GetScore()
{
	return m_Score;
}

void Character::SetLevel(int _level)
{
	m_Level = _level;
}

byte Character::GetLevel()
{
	return m_Level;
}

void Character::UpdateScore(int point)
{
	//�¸� ����Ʈ�̴�.
	if (point > 0)
	{
		//�̹� �ְ����̸� ó�� ���Ѵ�. 
		if (m_Level == MAX_LEVEL)
		{
			return;
		}

		if (m_Score.GetScorePoint() >= 75)
		{
			m_Level += 1;
			m_Score.SetScorePoint(0);
			return;
		}

		m_Score.AddScorePoint(point);

	}
	else
	{
		//���� ����Ʈ �̴�.
		if (m_Score.GetScorePoint() == 0)
		{
			if(m_Level > 1)
				m_Level -= 1;

			return;
		}

		m_Score.AddScorePoint(point);

		if (m_Score.GetScorePoint() < 0)
			m_Score.SetScorePoint(0);

	}

}