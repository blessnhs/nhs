#pragma once

#include "IGSContainer.h"
#include "./Player.h"

#include <boost/pool/object_pool.hpp>
#include <boost/shared_ptr.hpp>
#include <concurrent_unordered_map.h>

#include "TemplateStrategyPattern.h"


typedef boost::shared_ptr<Player> PlayerPtr;

using namespace std;

template<template<class T> class CreationPolicy>
class PlayerContainer : public IContainer<PlayerPtr>, public CreationPolicy<Player>
{
public:
	PlayerContainer();
	~PlayerContainer();

	PlayerPtr Create();

	bool Add(PlayerPtr &pObj);
	bool Del(PlayerPtr &pObj);

	bool Add2(PlayerPtr& pObj);
	bool Del2(PlayerPtr& pObj);

	PlayerPtr Search(INT64 Id);
	PlayerPtr Search(string flatformid);
	PlayerPtr SearchBySocketId(DWORD Id);
	PlayerPtr SearchByMachineId(string Id);


	PlayerPtr Search(DWORD FrontId,DWORD FrontSid);
	PlayerPtr SearchByFrontSid(DWORD FrontSid);

	void DelPlayerByFrontId(DWORD FrontId);

	void Print();

	VOID CheckUserList();

	int GetIncId() {return intAtomic.fetch_add(1);}

	int Count();

	template<class TYPE>
	VOID BroadCast(TYPE MSG);

	template<class TYPE>
	VOID BroadCast(TYPE MSG,INT64 INDEX);

	VOID OnResponse(LPVOID Data);

	VOID Disconnect(GSCLIENT_PTR pSession);
	VOID Disconnect(PlayerPtr pPlayer);

protected:
	atomic<int> intAtomic = 1;
	//���� concurrent_unordered_map �ڷ� ������ ĳ�̿뵵�� ������ �������� �ڷ� �����̱� ������
	//�������� ��ȸ�Ҷ��� ���� �������� �ϳ� �����.
	//m_PlayerMap erase ��� key value�� null�� �־��� �����̴�.
	concurrency::concurrent_unordered_map<INT64, PlayerPtr> m_PlayerMap;
	concurrency::concurrent_unordered_map<INT64, PlayerPtr> m_PlayerMapForLoop;

	CRITICAL_SECTION										  m_PublicLock;
};


extern PlayerContainer<GSCreator> &GetPlayerContainer();

#define PLAYERMGR GetPlayerContainer()

#include "PlayerContainer.hpp"


