#pragma once

class GSClient;

namespace GSNetwork	{	namespace IClientSession	{

/// @brief 클라이언트 전송과 연결 수명을 처리하는 콜백 인터페이스입니다.
class IClientSession
{
public:
	/// @brief 세션 인터페이스를 생성합니다.
	IClientSession(void);
	/// @brief 세션 인터페이스의 가상 소멸자입니다.
	virtual ~IClientSession(void);

	/// @brief 송신의 프로토콜과 데이터를 처리합니다.
	virtual void OnSend(WORD MainId,WORD SubId,char *Data,WORD Length) = 0;
	/// @brief 수신 바이트 수와 대상 세션을 받아 패킷 처리를 시작합니다.
	virtual void OnRecv(DWORD Length, boost::shared_ptr<GSClient> Client) = 0;
	/// @brief 대상 세션의 연결 종료를 처리합니다.
	virtual void OnDisconnect(boost::shared_ptr<GSClient>,bool isForce = false) = 0;
	/// @brief 대상 세션의 연결 승인 후 처리를 수행합니다.
	virtual void OnConnect(boost::shared_ptr<GSClient>) = 0;
};

}	}