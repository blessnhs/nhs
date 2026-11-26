#include "StdAfx.h"
#include "GSCrypt.h"
#include "GSSocket.h"
#include "GSPacket.h"

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{

GSPacket::GSPacket(VOID)
{
	m_Max_Packet_Buffer_Size = MAX_BUFFER_LENGTH * 5;
	m_PacketBuffer = new BYTE[m_Max_Packet_Buffer_Size];

	memset(m_PacketBuffer, 0, m_Max_Packet_Buffer_Size);	// [LOCKFIX] sizeof(포인터) -> 실제 버퍼 크기

	m_RemainLength			= 0;
	m_CurrentPacketNumber	= 0;

	m_LastReadPacketNumber	= 0;

	SetSocketType(SOCKET_NONE);
}

GSPacket::~GSPacket(VOID)
{
	delete[] m_PacketBuffer;		// [LOCKFIX] new[] 와 짝 맞춤
	m_PacketBuffer = NULL;

	m_PacketList.clear();
	m_WrietQueue.clear();

	////종료할때 비운다.
	//boost::shared_ptr<XDATA> pBuffer;
	//while (m_PacketList.empty() == false)
	//	m_PacketList.try_pop(pBuffer);

	//boost::shared_ptr<WRITE_PACKET_INFO> pWrite;
	//while (m_WrietQueue.empty() == false)
	//	m_WrietQueue.try_pop(pWrite);

}

BOOL GSPacket::Initialize(VOID)
{
	memset(m_PacketBuffer, 0, m_Max_Packet_Buffer_Size);	// [LOCKFIX] sizeof(포인터) -> 실제 버퍼 크기

	m_RemainLength			= 0;
	m_CurrentPacketNumber	= 0;
	m_LastReadPacketNumber	= 0;

#ifndef CLIENT_MODULE
	DrainQueue(m_WrietQueue);	// [LOCKFIX] 소켓 재사용 전(미완료 송신 없음) 호출. clear() 대신 스레드 안전한 drain
#endif
	return TRUE; 
}

BOOL GSPacket::Termination(VOID)
{	
	// [LOCKFIX] 여기서 m_WrietQueue를 비우지 않는다.
	// closesocket 직후에도 WSASend 완료 통지가 아직 오지 않은 버퍼가 있을 수 있는데,
	// 큐를 비우면 커널이 아직 쓰는 송신 버퍼가 해제된다. 남은 항목은 WriteComplete 또는 소멸자에서 해제된다.
	m_RemainLength			= 0;
	m_CurrentPacketNumber	= 0;
	m_LastReadPacketNumber	= 0;

	return TRUE;
}

BYTE *GSPacket::GetPacketBuffer()
{
	return m_PacketBuffer; 
}

BYTE  GSPacket::GetSocketType()
{
	return m_SocketType;	  
}

void GSPacket::SetSocketType(BYTE _Type)
{
	m_SocketType = _Type ; 
}


GSFactory<XDATA,true> & GSPacket::GetGSBufferPool()
{
	return m_GSBufferPool;
}

}	}	}