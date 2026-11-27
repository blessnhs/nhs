#pragma once

#include <mutex>
#include "Lock.h"

class ProxyHubContainer : public IContainer<GSCLIENT_PTR>, public Singleton<ProxyHubContainer>
{
public:

	ProxyHubContainer(void);
	~ProxyHubContainer(void);

	virtual bool Add(GSCLIENT_PTR& pObj);
	virtual bool Del(GSCLIENT_PTR& pObj);
	virtual GSCLIENT_PTR Search(INT64 Id);

	void TryConnect();

	void Create(INT Count = 1);

	GSCLIENT_PTR GetHub(unsigned long id);

private:
	CGSLock m_ProxyLock;
	CGSLock m_ConnectLock;
	concurrency::concurrent_unordered_map<int, GSCLIENT_PTR> m_Proxy_Map;
};

#define PROXYHUB Singleton<ProxyHubContainer>::Instance()