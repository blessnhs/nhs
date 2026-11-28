#pragma once

#include <memory>
#include <type_traits>

class GSClient;

enum IO_TYPE
{
	IO_ACCEPT,
	IO_READ,
	IO_WRITE,
	IO_UDP_READ
};

/// @brief IOCP가 사용하는 OVERLAPPED와 작업 종류, 클라이언트 ID 및 완료까지 유지되는 I/O 데이터를 보관합니다.
typedef struct _OVERLAPPED_EX
{
	/// @brief OVERLAPPED를 초기화하고 송신 버퍼 소유권을 비워 둡니다.
	_OVERLAPPED_EX()
	{
		memset(&Overlapped, 0, sizeof(OVERLAPPED));
		ObjectId = 0;
		SendBuffer = nullptr;
	}

	/// @brief 컨텍스트에 연결된 소유 송신 버퍼를 해제합니다.
	~_OVERLAPPED_EX()
	{
		delete[] SendBuffer;
	}

	_OVERLAPPED_EX(const _OVERLAPPED_EX&) = delete;
	_OVERLAPPED_EX& operator=(const _OVERLAPPED_EX&) = delete;

	OVERLAPPED	Overlapped;
	IO_TYPE		IoType;
	int			ObjectId;
	BYTE*       SendBuffer;
	SOCKADDR_IN SendAddress{};
	int ReceiveAddressLength = sizeof(SOCKADDR_IN);
	DWORD ReceiveFlags = 0;
} OVERLAPPED_EX;
static_assert(std::is_standard_layout<OVERLAPPED_EX>::value, "IOCP requires a stable OVERLAPPED prefix");

/// @brief 레거시 송신 큐에서 사용하는 소유 버퍼와 송신 메타데이터입니다.
class WRITE_PACKET_INFO
{
public:
	/// @brief 레거시 송신 버퍼와 길이, 시각 정보를 초기화합니다.
	WRITE_PACKET_INFO() { Data = NULL;  DataLength = 0;  Time = 0; }
	/// @brief 레거시 송신 큐 항목이 소유한 버퍼를 해제합니다.
	virtual ~WRITE_PACKET_INFO() 
	{
		if (Data != NULL)
			delete[] Data;

		Data = NULL;
	}

	BYTE	*Data;
	DWORD	DataLength;

	VOID	*Object;

	CHAR	RemoteAddress[14];
	USHORT	RemotePort;
	DWORD	PacketNumber;

	DWORD   Time;

};

/// @brief 클라이언트 모듈에서 사용하는 수신 버퍼와 패킷 메타데이터입니다.
class READ_PACKET_INFO
{
public:
	/// @brief 수신 큐 항목을 생성합니다.
	READ_PACKET_INFO(){}
	/// @brief 수신 큐 항목을 파괴합니다.
	virtual ~READ_PACKET_INFO() {}

	BYTE	Data[MAX_BUFFER_LENGTH];
	DWORD	DataLength;
	DWORD   Protocol;

	VOID	*Object;

	CHAR	RemoteAddress[14];
	USHORT	RemotePort;
	DWORD	PacketNumber;

};
