#pragma once

#include "GSSocketTCP.h"

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{	namespace GSPacketTCP	{

/// @brief TCP 바이트 스트림에서 프레임을 조립하고 압축 옵션을 포함한 송신 프레임을 구성합니다.
class GSPacketTCP : public GSNetwork::GSSocket::GSSocketTCP::GSSocketTCP, public GSNetwork::GSSocket::GSPacket::GSPacket
{
public:
	/// @brief TCP 패킷 처리기를 생성하고 소켓 유형을 설정합니다.
	GSPacketTCP(void);
	/// @brief TCP 패킷 처리기를 파괴합니다.
	virtual ~GSPacketTCP(void);

	/// @brief TCP 소켓과 패킷 조립 상태를 초기화합니다.
	BOOL	Initialize(VOID);
	/// @brief TCP 소켓과 패킷 상태 및 대기 패킷을 정리합니다.
	BOOL	Termination(VOID);
	
	/// @brief IOCP에서 받은 바이트를 TCP 프레임 조립 버퍼에 추가합니다.
	BOOL	ReadPacket(DWORD readLength);
	/// @brief 이벤트 기반 TCP 수신 데이터를 프레임 조립 버퍼에 추가합니다.
	BOOL	ReadPacketForEventSelect();
			
	//BOOL    SendPacking(LPVOID Packet,DWORD Size);
	//BOOL	Send(LPVOID Packet,DWORD Size);
	/// @brief 설정에 따라 압축 여부를 선택하여 TCP 프레임을 구성하고 송신합니다.
	BOOL	WritePacket(WORD MainProtocol,WORD SubProtocol, const BYTE *Packet, DWORD PayloadSize, long long Reserve2);

	/// @brief 페이로드를 압축하여 TCP 프레임으로 송신합니다.
	BOOL	WritePacketCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize,long long Reserve2 = 0);
	/// @brief 페이로드를 압축하지 않고 TCP 프레임으로 송신합니다.
	BOOL	WritePacketNoneCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize, long long Reserve2 = 0);
	/// @brief 전달받은 압축 플래그를 사용하여 페이로드를 TCP 프레임으로 중계합니다.
	BOOL	RelayPacket(WORD MainProtocol, WORD SubProtocol, BOOL Compress,const BYTE* Packet, DWORD PayloadSize, long long Reserve2 = 0);

	/// @brief TCP 송신 완료에 대한 패킷 계층의 후처리 훅입니다.
	BOOL	WriteComplete(VOID);

	// [LOCKFIX] 헤더 없이 원시 바이트 송신. 데이터를 힙에 복사하고 송신 큐에 등록해서 완료 시점까지 버퍼를 유지한다.
	/// @brief 프로토콜 헤더를 추가하지 않고 지정 바이트를 그대로 송신합니다.
	BOOL	WriteRaw(const BYTE* Data, DWORD Length);
	/// @brief 조립 버퍼에서 완성된 TCP 프레임 한 건을 파싱합니다.
	BOOL	GetPacket();

	/// @brief 수신 데이터를 누적한 뒤 완성된 TCP 프레임을 패킷 큐에 적재합니다.
	VOID	MakePacket(DWORD &dataLength);

	/// @brief TCP 패킷 파싱 후 메시지 생성에 사용하는 가상 훅입니다.
	virtual VOID MakeMsg(WORD Mainprotocol, WORD Subprotocol,DWORD dataLength);

	BOOL m_UseCompress;
	DWORD m_SequenceNum;

private:
	// [LOCKFIX] 조립이 끝난 버퍼를 송신 큐에 넣고 WSASend (이 구간만 락)
	/// @brief 완성된 송신 프레임을 기반 TCP 소켓에 전달합니다.
	BOOL	SendBuiltPacket(BYTE* SendBuff, DWORD PacketLength);

private:
	/// @brief 수신 바이트를 조립 버퍼에 추가합니다. 호출자가 수신 처리를 직렬화해야 합니다.
	BOOL ReadPacketLocked(DWORD readLength);
	/// @brief 완성된 프레임을 파싱합니다. 호출자가 수신 처리를 직렬화해야 합니다.
	BOOL GetPacketLocked();

};

}	}	}	}

#define GSPACKETTCP GSNetwork::GSSocket::GSPacket::GSPacketTCP::GSPacketTCP 