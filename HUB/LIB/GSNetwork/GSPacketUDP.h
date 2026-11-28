#pragma once

#include "GSSocketUDP.h"

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{	namespace GSPacketUDP	{

/// @brief UDP 데이터그램 단위로 헤더를 검증하고 프로토콜과 페이로드를 추출하거나 송신 프레임을 구성합니다.
class GSPacketUDP : public GSNetwork::GSSocket::GSSocketUDP::GSSocketUDP , public GSNetwork::GSSocket::GSPacket::GSPacket
{
public:
	/// @brief UDP 패킷 처리기를 생성하고 소켓 유형을 설정합니다.
	GSPacketUDP(void);
	/// @brief UDP 패킷 처리기를 파괴합니다.
	virtual ~GSPacketUDP(void);

	/// @brief UDP 소켓과 패킷 처리 상태를 초기화합니다.
	BOOL	Initialize(VOID);
	/// @brief UDP 패킷 상태, 대기 패킷과 소켓 자원을 정리합니다.
	BOOL	Termination(VOID);

	/// @brief 완료된 데이터그램과 송신자 정보를 패킷 파서로 넘깁니다.
	BOOL	ReadFromPacketForIocp(LPSTR& remoteAddress, USHORT &remotePort, DWORD readLength);
	/// @brief 이벤트 기반 수신에서 데이터그램 한 건을 받아 패킷 버퍼에 저장합니다.
	BOOL	ReadFromPacketForEventSelect(LPSTR remoteAddress, USHORT &remotePort);

	/// @brief UDP 길이, 프로토콜, 순번이 포함된 헤더를 작성한 뒤 데이터그램을 송신합니다.
	BOOL	WriteToPacket(LPCSTR remoteAddress, USHORT remotePort, WORD MainProtocol, WORD SubProtocol,const BYTE *packet, WORD packetLength);
	/// @brief UDP 송신 완료의 패킷 계층 훅입니다. 현재 구현은 성공을 반환합니다.
	BOOL	WriteComplete(VOID);

	/// @brief 데이터그램을 검증하고 정상 페이로드를 큐에 적재합니다.
	VOID	MakePacket(DWORD dwDataLength);
	/// @brief 정상 UDP 데이터그램에 대한 선택적 메시지 처리 훅입니다.
	virtual VOID MakeMsg(WORD Mainprotocol, WORD Subprotocol,DWORD dataLength) {};

	/// @brief UDP 수신 이력 상태를 초기화합니다.
	BOOL	ResetUdp(VOID);
	/// @brief 현재 데이터그램을 소비하고 헤더 정보와 실제 페이로드를 출력 버퍼에 복사합니다.
	BOOL	GetPacket(LPSTR remoteAddress, USHORT remotePort, WORD &MainProtocol, WORD &SubProtocol,BYTE *packet, DWORD &dataLength);

private:
	/// @brief 현재 데이터그램을 소비하며 선언 길이를 검증하고 프로토콜과 페이로드 길이를 추출합니다.
	BOOL ReadDatagramHeader(WORD& mainProtocol, WORD& subProtocol, DWORD& payloadLength);
};

}	}	}	}