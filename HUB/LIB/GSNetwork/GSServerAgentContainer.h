#pragma once

#include "GSClient.h"

namespace GSNetwork	{	
	
/// @brief 서버 간 연결을 등록·제거·조회하는 컨테이너 인터페이스입니다.
class IServerContainer
{
public:
	/// @brief 컨테이너 인터페이스의 기본 생성자입니다.
	IServerContainer()	 {}
	/// @brief 컨테이너 인터페이스의 기본 소멸자입니다.
	~IServerContainer() {}

	/// @brief 지정한 키에 서버 세션을 등록합니다.
	virtual bool Add(GSCLIENT_PTR pObj,DWORD Key) = 0;
	/// @brief 지정한 키의 서버 세션을 제거합니다.
	virtual bool Del(DWORD Key) = 0;

	/// @brief 지정 ID에 대응하는 서버 세션을 조회합니다.
	virtual GSCLIENT_PTR Search(DWORD Id) = 0;
};

}	