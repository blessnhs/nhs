#pragma once

class GSClient;

/// @brief 파싱된 TCP와 UDP 패킷을 처리하는 콜백 인터페이스입니다.
template<class T>
class IProcess
{
public:
	/// @brief 패킷 처리 인터페이스를 생성합니다.
	IProcess(void){}
	/// @brief 패킷 처리 인터페이스의 가상 소멸자입니다.
	virtual  ~IProcess(void){}

	/// @brief 대상 세션의 TCP 패킷을 처리합니다.
	virtual VOID Process(boost::shared_ptr<XDATA> pBuffer,boost::shared_ptr<GSClient> pClient) = 0;
	/// @brief 대상 포트의 UDP 패킷과 원격 주소·포트를 처리합니다.
	virtual VOID Process2(boost::shared_ptr<XDATA> pBuffer, boost::shared_ptr<GSClient> pClient, string remoteaddress, int remoteport) = 0;
	

	int pOwnerId;
};

