#include "StdAfx.h"
#include "GSPacketUDP.h"
#include "GSCrypt.h"
#include "GSPacket.h"

namespace { constexpr DWORD UdpHeaderSize = 3 * sizeof(WORD) + sizeof(DWORD); }

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{	namespace GSPacketUDP	{

	typedef GSNetwork::GSCrypt::GSCrypt	GSCrypt;

GSPacketUDP::GSPacketUDP(void)
{
	SetSocketType(SOCKET_UDP);
}

GSPacketUDP::~GSPacketUDP(void)
{
}

BOOL	GSPacketUDP::Initialize(VOID)
{
	CThreadSync Sync(*this);

	GSPacket::Initialize();

	return GSSocketUDP::Initialize();
}

BOOL	GSPacketUDP::Termination(VOID)
{
	CThreadSync Sync(*this);

	GSPacket::Termination();

#ifndef CLIENT_MODULE
	DrainQueue(m_PacketList);	// [LOCKFIX] concurrent_queue::clear()는 스레드 안전하지 않음
#endif
	return GSSocketUDP::Termination();
}


BOOL	GSPacketUDP::ReadFromPacketForIocp(LPSTR& remoteAddress, USHORT &remotePort, DWORD readLength)
{
	CThreadSync Sync(*this);

	if (!GSSocketUDP::ReadFromForIocp(remoteAddress, remotePort, m_PacketBuffer, readLength))
		return FALSE;

	m_RemainLength = readLength;

	//return getPacket(remoteAddress, remotePort, protocol, packet, packetLength);
	return TRUE;

}

BOOL	GSPacketUDP::ReadFromPacketForEventSelect(LPSTR remoteAddress, USHORT &remotePort)
{
	CThreadSync Sync(*this);

	DWORD ReadLength = 0;

	if (!GSSocketUDP::ReadFromForEventSelect(remoteAddress, remotePort, m_PacketBuffer, ReadLength))
		return FALSE;

	m_RemainLength = ReadLength;

	//return getPacket(remoteAddress, remotePort, protocol, packet, packetLength);
	return TRUE;

}

VOID	GSPacketUDP::MakePacket(DWORD ReadLength)
{
	CThreadSync sync(*this);
	LPSTR remoteAddress = nullptr;
	USHORT remotePort = 0;
	if (!ReadFromPacketForIocp(remoteAddress, remotePort, ReadLength))
	{
		InvalidDatagrams.fetch_add(1, std::memory_order_relaxed);
		return;
	}

	WORD mainProtocol = 0;
	WORD subProtocol = 0;
	DWORD length = 0;
	if (!ReadDatagramHeader(mainProtocol, subProtocol, length))
		return;

	auto packet = GetGSBufferPool().alloc();
	packet->m_Buffer.SetBuffer(m_PacketBuffer + UdpHeaderSize, length);
	packet->MainId = mainProtocol;
	packet->SubId = subProtocol;
	packet->Length = length;
	packet->LemoteAddress = remoteAddress;
	packet->RemotePort = remotePort;
	m_PacketList.push(packet);
	MakeMsg(mainProtocol, subProtocol, length);
}

BOOL	GSPacketUDP::WriteToPacket(LPCSTR remoteAddress, USHORT remotePort, WORD MainProtocol, WORD SubProtocol,const BYTE *packet, WORD packetLength)
{
	CThreadSync Sync(*this);

	if (!packet)
		return FALSE;

	const DWORD wireLength = UdpHeaderSize + packetLength;
	if (wireLength > MAX_BUFFER_LENGTH || wireLength > MAXWORD)
		return FALSE;
	const WORD PacketLength = static_cast<WORD>(wireLength);

	m_CurrentPacketNumber++;

	BYTE TempBuffer[MAX_BUFFER_LENGTH];
	
	memcpy(TempBuffer, &PacketLength, sizeof(WORD));
	
	memcpy(TempBuffer + 
		sizeof(WORD), 
		&MainProtocol, sizeof(WORD));
	
	memcpy(TempBuffer + 
		sizeof(WORD)  + 
		sizeof(WORD), 
		&SubProtocol, sizeof(WORD));

	memcpy(TempBuffer + 
		sizeof(WORD)  + 
		sizeof(WORD)  + 
		sizeof(WORD), 
		&m_CurrentPacketNumber, sizeof(DWORD));

	memcpy(TempBuffer + 
		sizeof(WORD)  + 
		sizeof(WORD)  +
		sizeof(WORD)  + 
		sizeof(DWORD),
		packet, packetLength);

//	GSCrypt::Encrypt(TempBuffer + sizeof(DWORD), TempBuffer + sizeof(DWORD), PacketLength - sizeof(DWORD));

	// WriteQueue를 이용해서 패킷이 전송 완료가 되었을까지 메모리를 살려둔다.

	return GSSocketUDP::WriteTo2((LPSTR)remoteAddress, remotePort, TempBuffer, PacketLength);

//	return GSSocketUDP::WriteTo(remoteAddress, remotePort, TempBuffer, PacketLength);
}

BOOL GSPacketUDP::WriteComplete(VOID)
{
	return TRUE;
}

BOOL	GSPacketUDP::ResetUdp(VOID)
{
	CThreadSync	Sync(*this);

	m_LastReadPacketInfoVectorForUdp.clear();

	return TRUE;

}

BOOL	GSPacketUDP::GetPacket(LPSTR remoteAddress, USHORT remotePort,WORD &MainProtocol, WORD &SubProtocol,BYTE *packet, DWORD &dataLength)
{
	CThreadSync sync(*this);
	if (!packet || !ReadDatagramHeader(MainProtocol, SubProtocol, dataLength))
		return FALSE;
	memcpy(packet, m_PacketBuffer + UdpHeaderSize, dataLength);
	return TRUE;
}

BOOL GSPacketUDP::ReadDatagramHeader(WORD& MainProtocol, WORD& SubProtocol, DWORD& dataLength)
{
	const DWORD received = m_RemainLength;
	m_RemainLength = 0;
	WORD declared = 0;
	if (received >= UdpHeaderSize)
		memcpy(&declared, m_PacketBuffer, sizeof(declared));
	if (received < UdpHeaderSize || declared != received)
	{
		InvalidDatagrams.fetch_add(1, std::memory_order_relaxed);
		return FALSE;
	}

	memcpy(&MainProtocol, m_PacketBuffer + sizeof(WORD), sizeof(MainProtocol));
	memcpy(&SubProtocol, m_PacketBuffer + 2 * sizeof(WORD), sizeof(SubProtocol));
	dataLength = received - UdpHeaderSize;
	return TRUE;
}

}	}	}	}