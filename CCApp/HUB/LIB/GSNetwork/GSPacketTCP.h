#pragma once

#include "GSSocketTCP.h"

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{	namespace GSPacketTCP	{

class GSPacketTCP : public GSNetwork::GSSocket::GSSocketTCP::GSSocketTCP, public GSNetwork::GSSocket::GSPacket::GSPacket
{
public:
	GSPacketTCP(void);
	virtual ~GSPacketTCP(void);

	BOOL	Initialize(VOID);
	BOOL	Termination(VOID);
	
	BOOL	ReadPacket(DWORD readLength);
	BOOL	ReadPacketForEventSelect();
			
	//BOOL    SendPacking(LPVOID Packet,DWORD Size);
	//BOOL	Send(LPVOID Packet,DWORD Size);
	BOOL	WritePacket(WORD MainProtocol,WORD SubProtocol, const BYTE *Packet, DWORD PayloadSize, long long Reserve2);

	BOOL	WritePacketCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize,long long Reserve2 = 0);
	BOOL	WritePacketNoneCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize, long long Reserve2 = 0);
	BOOL	RelayPacket(WORD MainProtocol, WORD SubProtocol, BOOL Compress,const BYTE* Packet, DWORD PayloadSize, long long Reserve2 = 0);

	BOOL	WriteComplete(VOID);

	// [LOCKFIX] 헤더 없이 원시 바이트 송신. 데이터를 힙에 복사하고 송신 큐에 등록해서 완료 시점까지 버퍼를 유지한다.
	BOOL	WriteRaw(const BYTE* Data, DWORD Length);
	BOOL	GetPacket();

	VOID	MakePacket(DWORD &dataLength);

	virtual VOID MakeMsg(WORD Mainprotocol, WORD Subprotocol,DWORD dataLength);

	BOOL m_UseCompress;
	DWORD m_SequenceNum;

private:
	// [LOCKFIX] 조립이 끝난 버퍼를 송신 큐에 넣고 WSASend (이 구간만 락)
	BOOL	SendBuiltPacket(BYTE* SendBuff, DWORD PacketLength);

};

}	}	}	}

#define GSPACKETTCP GSNetwork::GSSocket::GSPacket::GSPacketTCP::GSPacketTCP 