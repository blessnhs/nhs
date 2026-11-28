#pragma once

#include <concurrent_unordered_map.h>

#include "IManager.h"
#include "GSServerAgentContainer.h"
#include "SingleTonHolder.h"
#include "GSClient.h"

namespace GSNetwork	{	
		
/// @brief 서버 간 연결의 GSClient 참조를 ID로 보관하는 컨테이너입니다.
class GSServerAgent : public IServerContainer , public Singleton<GSServerAgent>
{
public:
	/// @brief 서버 연결 컨테이너를 생성합니다.
	GSServerAgent(void);
	/// @brief 서버 연결 컨테이너를 파괴합니다.
	~GSServerAgent(void);

	/// @brief 키에 연결된 서버 세션을 등록합니다.
	bool Add(GSCLIENT_PTR pObj,DWORD Key);
	/// @brief 지정 키의 서버 세션을 제거합니다.
	bool Del(DWORD Key);

	/// @brief 지정 ID에 등록된 서버 세션을 조회합니다.
	GSCLIENT_PTR Search(DWORD Id);

	concurrency::concurrent_unordered_map<DWORD, GSCLIENT_PTR> m_ServerMap;
};

}	

#define AGT_SVR Singleton<GSServerAgent>::Instance()