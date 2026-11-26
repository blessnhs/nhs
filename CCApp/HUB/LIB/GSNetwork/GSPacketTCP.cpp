#include "StdAfx.h"
#include "GSPacketTCP.h"
#include "GSCrypt.h"
#include "CommonSock.h"
#include "GSAllocator.h"
#include "GSMainProcess.h"
#include "MSG_PLAYER_PACKET.h"
#include "lzf.h"

namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{	namespace GSPacketTCP	{

GSPacketTCP::GSPacketTCP(void)
{
	SetSocketType(SOCKET_TCP);
	m_SequenceNum = 0;
}

GSPacketTCP::~GSPacketTCP(void)
{
}

BOOL	GSPacketTCP::Initialize(VOID)
{
	CThreadSync Sync(this);

	GSPacket::Initialize();

	return GSSocketTCP::Initialize();
}

BOOL	GSPacketTCP::Termination(VOID)
{
		
	CThreadSync Sync(this);

	GSPacket::Termination();
#ifndef CLIENT_MODULE
	DrainQueue(m_PacketList);	// [LOCKFIX] concurrent_queue::clear()는 스레드 안전하지 않음
#endif

	return GSSocketTCP::Termination();

}

// [LOCKFIX] 수신 경로(MakePacket/ReadPacket/GetPacket)는 락을 잡지 않는다.
// IOCP에서 소켓마다 미완료 WSARecv는 항상 1개뿐이라(완료 처리 후 다시 InitializeReadForIocp)
// m_PacketBuffer / m_RemainLength는 한 시점에 한 스레드만 접근한다.
// 덕분에 gzip 해제 중에도 같은 소켓의 송신이 막히지 않는다.
BOOL	GSPacketTCP::ReadPacket(DWORD ReadSize)
{
	if (m_Max_Packet_Buffer_Size <= (ReadSize +m_RemainLength))
	{
		DWORD Resize = ReadSize + m_RemainLength + m_Max_Packet_Buffer_Size;
		BYTE* NewBuff = new BYTE[Resize];
		memcpy(NewBuff, m_PacketBuffer, m_Max_Packet_Buffer_Size);
		delete[] m_PacketBuffer;		// [LOCKFIX] new[] 와 짝 맞춤

		m_PacketBuffer = NewBuff;

		m_Max_Packet_Buffer_Size = Resize;

		SYSLOG().Write("ReadPacket Resize %d bytes %d kbytes %d mbytes\n", Resize,Resize/1024,Resize/1024/1024);
	}

	if (!GSSocketTCP::ReadForIocp(m_PacketBuffer, ReadSize, m_RemainLength, m_Max_Packet_Buffer_Size))
	{

		return FALSE;
	}

	m_RemainLength	+= ReadSize;

	//return getPacket(protocol, packet, packetLength);
	return TRUE;
}

BOOL	GSPacketTCP::ReadPacketForEventSelect()
{
	CThreadSync Sync(this);

	DWORD ReadLength = 0;

	if (!GSSocketTCP::ReadForEventSelect(m_PacketBuffer + m_RemainLength, ReadLength))
		return FALSE;

	m_RemainLength	+= ReadLength;

	//return getPacket(protocol, packet, packetLength);
	return TRUE;
}

//BOOL    GSPacketTCP::SendPacking(LPVOID pData, DWORD dwbytes)
//{
//	char pTemp[4096] = {0,};
//	DWORD index=0;
//
//	pTemp[index++] = (BYTE)_PACKET_START1;
//	pTemp[index++] = (BYTE)_PACKET_START2;
//	memcpy( pTemp+index, &dwbytes, 2 );	// unsigned short
//	index += 2;
//	memcpy( pTemp+index, pData, dwbytes );
//	index += dwbytes;
//	pTemp[index++] = (BYTE)_PACKET_END1;
//	pTemp[index++] = (BYTE)_PACKET_END2;
//
////	SYSLOG().Write("socket to : %d, pid : %d \n",Data->FROM, Data->MPID);
//
//	return (Send(pTemp, index));
//}
//
//BOOL	GSPacketTCP::Send(LPVOID Packet,DWORD Size)
//{
//	CThreadSync Sync;
//
//	if (!Packet)
//		return FALSE;
//	
//	boost::shared_ptr<WRITE_PACKET_INFO> pWriteData(m_WritePool.alloc());
//
//	CopyMemory(pWriteData->Data,Packet,sizeof(pWriteData->Data));
//	pWriteData->DataLength = Size;
//	pWriteData->Object = this;
//#ifndef CLIENT_MODULE	
//	m_WrietQueue.push(pWriteData);
//#endif
//	return GSSocketTCP::Write((BYTE*)pWriteData->Data, Size);
//}
// 
// 
// 
// 

//2022 06 24 extend protocol header
// size 4 + main protocol id 2 + sub protocol id 2 + sequence 4 + reserve1 1 + reserver2 8
//
// [LOCKFIX] 2026-11-26
//  이전: 헤더 조립(new + memcpy 7회)과 gzip 압축을 소켓 락 안에서 수행 (게다가 그 락이 전역이었음)
//  변경: 버퍼 조립/압축은 락 없이 하고, 락은 "송신 큐 push + WSASend" 순서를 맞추는 구간에만 잡는다.
//        WriteComplete()가 큐를 FIFO로 pop하므로 push 순서와 WSASend 순서가 같아야 한다.

static const DWORD TCP_PACKET_HEADER_SIZE =
	sizeof(DWORD) +			// size 4
	sizeof(WORD) +			// main protocol id 2
	sizeof(WORD) +			// sub protocol id 2
	sizeof(DWORD) +			// sequence 4
	sizeof(BYTE) +			// reserve1 1 (compress flag)
	sizeof(long long);		// reserve2 8

static const DWORD TCP_PACKET_SEQUENCE_OFFSET = sizeof(DWORD) + sizeof(WORD) + sizeof(WORD);

// 락 없이 호출: 지역 데이터만 사용
static BYTE* BuildTcpPacket(WORD MainProtocol, WORD SubProtocol, BYTE CompressFlag, const BYTE* Payload, DWORD PayloadSize, long long Reserve2, DWORD& OutLength)
{
	DWORD PacketLength = TCP_PACKET_HEADER_SIZE + PayloadSize;
	DWORD Sequence = 0;

	BYTE* sendBuff = new BYTE[PacketLength];
	BYTE* p = sendBuff;

	memcpy(p, &PacketLength, sizeof(DWORD));	p += sizeof(DWORD);
	memcpy(p, &MainProtocol, sizeof(WORD));		p += sizeof(WORD);
	memcpy(p, &SubProtocol, sizeof(WORD));		p += sizeof(WORD);
	memcpy(p, &Sequence, sizeof(DWORD));		p += sizeof(DWORD);		// 실제 값은 SendBuiltPacket에서 락 안에서 기록
	memcpy(p, &CompressFlag, sizeof(BYTE));		p += sizeof(BYTE);
	memcpy(p, &Reserve2, sizeof(long long));	p += sizeof(long long);

	if (PayloadSize > 0)
		memcpy(p, Payload, PayloadSize);

	//GSCrypt::Encrypt(TempBuffer + sizeof(WORD), TempBuffer + sizeof(WORD), PacketLength - sizeof(WORD));

	OutLength = PacketLength;
	return sendBuff;
}

BOOL	GSPacketTCP::SendBuiltPacket(BYTE* SendBuff, DWORD PacketLength)
{
	boost::shared_ptr<WRITE_PACKET_INFO> pWriteData;
	try
	{
		pWriteData.reset(new WRITE_PACKET_INFO);
	}
	catch (...)
	{
		delete[] SendBuff;
		throw;
	}

	pWriteData->Data = SendBuff;			// 소유권 이전 (WRITE_PACKET_INFO 소멸자에서 delete[])
	pWriteData->DataLength = PacketLength;

	CThreadSync Sync(this);

	memcpy(SendBuff + TCP_PACKET_SEQUENCE_OFFSET, &m_SequenceNum, sizeof(DWORD));

#ifndef CLIENT_MODULE	
	m_WrietQueue.push(pWriteData);
#endif	

	return GSSocketTCP::Write(pWriteData);
}

BOOL	GSPacketTCP::WriteRaw(const BYTE* Data, DWORD Length)
{
	if (!Data || Length == 0)
		return FALSE;

	boost::shared_ptr<WRITE_PACKET_INFO> pWriteData(new WRITE_PACKET_INFO);
	pWriteData->Data = new BYTE[Length];
	pWriteData->DataLength = Length;
	memcpy(pWriteData->Data, Data, Length);

	CThreadSync Sync(this);

#ifndef CLIENT_MODULE	
	m_WrietQueue.push(pWriteData);
#endif	

	return GSSocketTCP::Write(pWriteData);
}

BOOL	GSPacketTCP::WritePacketNoneCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize, long long Reserve2)
{
	if (!Packet)
		return FALSE;

	DWORD PacketLength = 0;
	BYTE* sendBuff = BuildTcpPacket(MainProtocol, SubProtocol, 0, Packet, PayloadSize, Reserve2, PacketLength);

	return SendBuiltPacket(sendBuff, PacketLength);
}

BOOL	GSPacketTCP::RelayPacket(WORD MainProtocol, WORD SubProtocol, BOOL Compress, const BYTE* Packet, DWORD PayloadSize, long long Reserve2)
{
	if (!Packet)
		return FALSE;

	DWORD PacketLength = 0;
	BYTE* sendBuff = BuildTcpPacket(MainProtocol, SubProtocol, (BYTE)Compress, Packet, PayloadSize, Reserve2, PacketLength);

	return SendBuiltPacket(sendBuff, PacketLength);
}


BOOL	GSPacketTCP::WritePacketCompress(WORD MainProtocol, WORD SubProtocol, const BYTE* Packet, DWORD PayloadSize, long long Reserve2)
{
	if (!Packet)
		return FALSE;

	//compress (락 밖)
	std::string src((char*)Packet, PayloadSize);

	std::string out = Gzip::compress(src);

	DWORD PacketLength = 0;
	BYTE* sendBuff = BuildTcpPacket(MainProtocol, SubProtocol, 1, (const BYTE*)out.c_str(), (DWORD)out.size(), Reserve2, PacketLength);

	return SendBuiltPacket(sendBuff, PacketLength);
}

BOOL	GSPacketTCP::WritePacket(WORD MainProtocol,WORD SubProtocol, const BYTE * Packet, DWORD PayloadSize, long long Reserve2)
{
	if (PayloadSize > 760)
		return WritePacketCompress(MainProtocol, SubProtocol, Packet, PayloadSize, Reserve2);
	else
		return WritePacketNoneCompress(MainProtocol, SubProtocol, Packet, PayloadSize, Reserve2);

}

BOOL GSPacketTCP::WriteComplete(VOID)
{
#ifndef CLIENT_MODULE
	if(m_WrietQueue.unsafe_size() == 0) return FALSE;

	boost::shared_ptr<WRITE_PACKET_INFO> pWriteData;

	if (m_WrietQueue.try_pop(pWriteData) == true)
	{
	}


#endif	

	

	return TRUE;
}

VOID GSPacketTCP::MakePacket(DWORD &dwPacketLength )
{
	if (ReadPacket(dwPacketLength))
	{
		while (GetPacket())
		{
			
		}
	}
}

VOID GSPacketTCP::MakeMsg(WORD Mainprotocol, WORD Subprotocol,DWORD dataLength)
{
}

BOOL GSPacketTCP::GetPacket()
{
	if (m_RemainLength <= sizeof(DWORD))
		return FALSE;

	//헤더를 제외한 실제 데이터 사이즈
	DWORD PayLoadLength;

	//패킷 헤더를 포함한 전체 사이즈
	DWORD PacketLength = 0;
	memcpy(&PacketLength, m_PacketBuffer, sizeof(DWORD));

	if (PacketLength >= LMIT_BUFFER_LENGTH || PacketLength <= 0) 
	{
		SYSLOG().Write("!GetPacket Packet Size Wrong %d\n", PacketLength);
		m_RemainLength = 0;
		return FALSE;
	}

	//2020.10.13 서버 덤프없이 사라지는 버그 패킷 전송시 클라이언트에서 Size패킷을 변조해 헤더보다 작은 값으로 보내서
	//복사하다 죽었다.
	//4바이트 (크기) + 2바이트(protocolid)  + 2바이트 (sub protocolid) + 4바이트(sequence number) + 1바이트(압축여부) reserver1 + 8바이트 reserver2
	if (PacketLength < (sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD) + sizeof(byte) + sizeof(long long) ))
	{
		SYSLOG().Write("!!GetPacket Packet Size Wrong %d\n", PacketLength);
		return FALSE;
	}

	if (PacketLength <= m_RemainLength)			
	{
	//	GSCrypt::Decrypt(m_PacketBuffer + sizeof(WORD), 
	//		m_PacketBuffer + sizeof(WORD), 
	//		PacketLength   - sizeof(WORD));

	
		WORD MainProtocol = 0;
		WORD SubProtocol = 0;
		DWORD SequenceNumber = 0;
		byte CompressFlag     = 0;

		long long Reserve2;

		memcpy(&MainProtocol   , m_PacketBuffer + sizeof(DWORD),   sizeof(WORD));
		memcpy(&SubProtocol    , m_PacketBuffer + sizeof(DWORD)  + sizeof(WORD) , sizeof(WORD));
		memcpy(&SequenceNumber, m_PacketBuffer + sizeof(DWORD)  + sizeof(WORD)  + sizeof(WORD), sizeof(DWORD));

		memcpy(&CompressFlag, m_PacketBuffer + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD), sizeof(byte));
		memcpy(&Reserve2, m_PacketBuffer + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD) + sizeof(byte), sizeof(long long));
		
		PayLoadLength = PacketLength - sizeof(DWORD) - sizeof(WORD) - sizeof(WORD) - sizeof(DWORD) - sizeof(byte) - sizeof(long long);

		boost::shared_ptr<XDATA> pBuffer(m_GSBufferPool.alloc());

		if(CompressFlag == 1 && m_UseCompress == true)
		{
			std::string src((char *)(m_PacketBuffer + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD) + sizeof(byte) + sizeof(long long)), PayLoadLength);

			std::string out = Gzip::decompress(src);

			pBuffer->m_Buffer.SetBuffer((BYTE *)out.c_str(), out.size());
		}
		else
		{
			pBuffer->m_Buffer.SetBuffer(m_PacketBuffer + sizeof(DWORD) + sizeof(WORD) + sizeof(WORD) + sizeof(DWORD) + sizeof(byte) + sizeof(long long), PayLoadLength);
		}
	
		pBuffer->IsCompress = CompressFlag;
		pBuffer->MainId = MainProtocol;
		pBuffer->SubId = SubProtocol;
		pBuffer->Reserve2 = Reserve2;
		pBuffer->Length = pBuffer->m_Buffer.GetLength();

		m_PacketList.push(pBuffer);


		if (m_RemainLength - PacketLength > 0)
			memmove(m_PacketBuffer, m_PacketBuffer + PacketLength, m_RemainLength - PacketLength);

		m_RemainLength -= PacketLength;

		if (m_RemainLength <= 0)
		{
			m_RemainLength = 0;
			// [LOCKFIX] memset(m_PacketBuffer, 0, sizeof(m_PacketBuffer)) 제거: 포인터 크기(8바이트)만 지우던 코드이고
			// 유효 데이터 범위는 m_RemainLength로 관리하므로 지울 필요가 없다.
		}

	//	MakeMsg(MainProtocol, SubProtocol, PayLoadLength);

		return TRUE;

	}

	return FALSE;
}

}	}	}	}