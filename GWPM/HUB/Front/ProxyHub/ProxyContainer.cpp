#include "stdafx.h"
#include "ProxyContainer.h"
#include "../server/hubProcess.h"

ProxyHubContainer::ProxyHubContainer(void)
{

}

ProxyHubContainer::~ProxyHubContainer(void)
{
}

bool ProxyHubContainer::Add(GSCLIENT_PTR& pObj)
{
	std::lock_guard<CGSLock> lock(m_ProxyLock);
	m_Proxy_Map[pObj->GetId()] = pObj;

	return TRUE;
}

bool ProxyHubContainer::Del(GSCLIENT_PTR& pObj)
{
	std::lock_guard<CGSLock> lock(m_ProxyLock);
	auto iter = m_Proxy_Map.find(pObj->GetId());
	if (iter == m_Proxy_Map.end() || iter->second != pObj)
		return false;

	m_Proxy_Map.unsafe_erase(iter);
	return true;
}

GSCLIENT_PTR ProxyHubContainer::Search(INT64 Id)
{
	std::lock_guard<CGSLock> lock(m_ProxyLock);
	auto iter = m_Proxy_Map.find(Id);
	if (iter != m_Proxy_Map.end())
	{
		return iter->second;
	}

	return GSCLIENT_PTR();
}

GSCLIENT_PTR ProxyHubContainer::GetHub(unsigned long id)
{
	std::lock_guard<CGSLock> lock(m_ProxyLock);
	std::vector< GSCLIENT_PTR> vt;

	for (const auto& proxy : m_Proxy_Map)
	{
		if (proxy.second->GetConnected() == false)
			continue;

		vt.push_back(proxy.second);
	}

	if(vt.size() == 0)
		return GSCLIENT_PTR();
	else
	{
		return vt[id % vt.size()];
	}
}


void ProxyHubContainer::Create(INT Count)
{
	GSServer::GSServer* pServer = (GSServer::GSServer*)&SERVER;

	for (int i = 0; i < Count; i++)
	{
		auto pClient = pServer->GetClientMgr().NewClient2();
		if (pClient == NULL)
		{
			SYSLOG().Write(" ProxyHubContainer::Initialize failed...1 \n");
			break;
		}


		pClient->SetHandler<HubProcess>();
		Add(pClient);
	}

	TryConnect();

}


void ProxyHubContainer::TryConnect()
{
	std::unique_lock<CGSLock> connectLock(m_ConnectLock, std::try_to_lock);
	if (!connectLock.owns_lock())
		return;

	std::vector<GSCLIENT_PTR> clients;
	{
		std::lock_guard<CGSLock> lock(m_ProxyLock);
		for (const auto& proxy : m_Proxy_Map)
			clients.push_back(proxy.second);
	}

	for (const auto& client : clients)
	{
		if (client->GetConnected() || Search(client->GetId()) != client)
			continue;

		auto socket = client->GetTCPSocket();
		const SOCKET handle = socket->Connect2(INI.HubIp, INI.HubPort);
		if (!handle)
		{
			socket->Termination();
			SYSLOG().Write("ProxyHubContainer: HUB connect failed\n");
			continue;
		}

		if (!socket->RegisterIocp(SERVER, static_cast<ULONG_PTR>(client->GetId()) + 1) ||
			!socket->Initialize())
		{
			socket->Termination();
			SYSLOG().Write("ProxyHubContainer: HUB socket setup failed\n");
			continue;
		}

		// A first completion can arrive as soon as the read is submitted.
		// Publish readiness before an IOCP disconnect handler can inspect it.
		bool readStarted = false;
		{
			GSCLIENT::CThreadSync clientLock;
			if (Search(client->GetId()) == client && client->InitializeReadForIocp())
			{
				client->SetConnected(true);
				readStarted = true;
			}
		}
		if (!readStarted)
		{
			socket->Termination();
			SYSLOG().Write("ProxyHubContainer: HUB read registration failed\n");
		}
	}
}
