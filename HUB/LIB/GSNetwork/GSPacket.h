#pragma once



#include "./GSFactory.h"
#include <concurrent_queue.h>
#include "GSBuffer.h"



namespace GSNetwork	{ namespace GSSocket	{	namespace GSPacket	{

/// @brief 파싱된 패킷의 프로토콜, 플래그, 주소 및 페이로드 버퍼를 보관합니다.
struct XDATA
{
	/// @brief 파싱 결과를 담을 패킷 항목을 생성합니다.
	XDATA() {}
	/// @brief 파싱된 패킷 항목을 파괴합니다.
	~XDATA()
	{
	}
	XBUFFER m_Buffer;
	DWORD Length;
	WORD MainId;
	WORD SubId;
	BOOL IsCompress;//(Reserve1)
	long long Reserve2;

	//udp만 필요
	string LemoteAddress;
	WORD RemotePort;
};

// [LOCKFIX] concurrent_queue::clear()는 다른 스레드의 push/try_pop과 동시에 호출하면 안 된다.
// try_pop으로 비우면 동시 접근 중에도 안전하다.
/// @brief 동시 큐에서 얻을 수 있는 항목을 꺼내 각 항목의 참조를 해제합니다.
template<class Q>
inline void DrainQueue(Q& q)
{
	typename Q::value_type item;
	while (q.try_pop(item)) {}
}

/// @brief TCP와 UDP가 공유하는 패킷 버퍼, 상태 및 객체 풀 인터페이스를 제공합니다.
class GSPacket 
{
public:
	/// @brief 패킷 조립에 사용하는 내부 버퍼를 할당합니다.
	GSPacket(VOID);
	/// @brief 패킷 조립에 사용한 내부 버퍼를 해제합니다.
	virtual ~GSPacket(VOID);

	enum _SocketType
	{
		SOCKET_TCP,
		SOCKET_UDP,
		SOCKET_NONE,
	};


protected:
	//받은 패킷을 저장하는 사이즈					
	DWORD																	m_Max_Packet_Buffer_Size;
	//받은 패킷을 저장하는 버퍼 //패킷이 오면 큐에 넣어서꺼내기까지 몇개든지 이 버퍼에 저장					
	BYTE																	*m_PacketBuffer;
	//처리하고 남은 사이즈 혹은 다 받지 못한 패킷사이즈
	DWORD																	m_RemainLength;

	DWORD																	m_CurrentPacketNumber;
	DWORD																	m_LastReadPacketNumber;

	BYTE																	m_SocketType;

	GSFactory<XDATA,true>													m_GSBufferPool;

	GSFactory<WRITE_PACKET_INFO,true>										m_WritePool;
	std::vector<READ_PACKET_INFO>											m_LastReadPacketInfoVectorForUdp;
	
	/// @brief 파싱된 XDATA 항목을 할당하는 객체 풀을 반환합니다.
	GSFactory<XDATA,true> &													GetGSBufferPool();
public:
#ifndef CLIENT_MODULE
	Concurrency::concurrent_queue< boost::shared_ptr<WRITE_PACKET_INFO> >			m_WrietQueue;
	Concurrency::concurrent_queue< boost::shared_ptr<XDATA> >						m_PacketList;
#endif

public:
	/// @brief 내부 패킷 버퍼와 조립 상태의 초기값을 설정합니다.
	BOOL	Initialize(VOID);
	/// @brief 패킷 조립 및 시퀀스 관련 상태를 정리합니다.
	BOOL	Termination(VOID);

	/// @brief 내부 패킷 조립 버퍼의 시작 주소를 반환합니다.
	BYTE    *GetPacketBuffer();
	/// @brief 패킷 처리기의 소켓 유형을 반환합니다.
	BYTE    GetSocketType();
	/// @brief 패킷 처리기의 소켓 유형을 설정합니다.
	void    SetSocketType(BYTE _Type);

};

}	}	}

typedef GSNetwork::GSSocket::GSPacket::XDATA XDATA;