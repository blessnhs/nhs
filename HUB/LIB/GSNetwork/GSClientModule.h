#pragma once

#include "GSEventSelect.h"

#include "GSPacketTCP.h"
#include "GSPacketUDP.h"

namespace GSNetwork { namespace GSEventSelect {	namespace GSClientModule	{

typedef GSNetwork::GSSocket::GSPacket::GSPacketTCP::GSPacketTCP GSPacketTCP;
typedef GSNetwork::GSSocket::GSPacket::GSPacketUDP::GSPacketUDP GSPacketUDP;

/// @brief WSAEventSelect를 사용하는 클라이언트용 TCP·UDP 수신과 패킷 큐를 제공합니다.
class GSClientModule : public GSEventSelect
{
public:
	/// @brief 클라이언트 모듈의 초기 상태를 설정합니다.
	GSClientModule(VOID);
	/// @brief 클라이언트 모듈 객체를 파괴합니다.
	virtual ~GSClientModule(VOID);

private:
	GSPacketTCP	mTCPSession;
	GSPacketUDP	mUDPSession;


	std::queue<std::shared_ptr<READ_PACKET_INFO>>		m_ReadPacketQueue;					

	BOOL			IsUdp;

public:
	/// @brief 원격 서버로 TCP 연결을 맺고 소켓 이벤트 처리를 시작합니다.
	BOOL			BeginTcp(LPSTR remoteAddress, USHORT remotePort);
	/// @brief 지정 로컬 포트에 UDP 소켓을 바인딩하고 소켓 이벤트 처리를 시작합니다.
	BOOL			BeginUdp(USHORT remotePort);

	/// @brief 클라이언트 모듈의 소켓 이벤트와 전송 상태를 정리합니다.
	BOOL			End(VOID);

	/// @brief 현재 전송 방식의 로컬 IP 조회 성공 여부를 반환합니다. 문자열 인수는 값으로 전달됩니다.
	BOOL			GetLocalIP(string pIP);
	/// @brief 현재 전송 방식의 로컬 포트를 반환합니다.
	USHORT			GetLocalPort(VOID);

	/// @brief 수신 큐의 첫 패킷을 호출자 버퍼에 복사합니다. 버퍼는 MAX_BUFFER_LENGTH 바이트를 담을 수 있어야 합니다.
	BOOL			ReadPacket(WORD &protocol, BYTE *packet, WORD &packetLength);

	/// @brief TCP 패킷을 작성하여 전송하고 송신 후처리를 호출합니다.
	BOOL			WritePacket(WORD protocol, WORD Subprotocol,const BYTE *packet, WORD packetLength);

protected:
	/// @brief 읽기 이벤트를 받아 현재 전송 방식에 맞춰 파싱하고 수신 큐에 넣습니다.
	VOID			OnIoRead(VOID);
};

}	}	}