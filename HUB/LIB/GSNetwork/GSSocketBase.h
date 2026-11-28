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

typedef struct _OVERLAPPED_EX
{
	_OVERLAPPED_EX()
	{
		memset(&Overlapped, 0, sizeof(OVERLAPPED));
		ObjectId = 0;
		SendBuffer = nullptr;
	}

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

class WRITE_PACKET_INFO
{
public:
	WRITE_PACKET_INFO() { Data = NULL;  DataLength = 0;  Time = 0; }
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

class READ_PACKET_INFO
{
public:
	READ_PACKET_INFO(){}
	virtual ~READ_PACKET_INFO() {}

	BYTE	Data[MAX_BUFFER_LENGTH];
	DWORD	DataLength;
	DWORD   Protocol;

	VOID	*Object;

	CHAR	RemoteAddress[14];
	USHORT	RemotePort;
	DWORD	PacketNumber;

};
