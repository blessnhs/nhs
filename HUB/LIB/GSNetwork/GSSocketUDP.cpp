#include "StdAfx.h"
#include "GSSocketUDP.h"
#include <winsock2.h>
#include <utility>

namespace GSNetwork	{ namespace GSSocket	{	namespace GSSocketUDP	{

DWORD WINAPI ReliableUdpThreadCallback(LPVOID parameter)
{
	GSSocketUDP *Owner = (GSSocketUDP*) parameter;
	Owner->ReliableUdpThreadCallback();

	return 0;
}

GSSocketUDP::GSSocketUDP(void)
{
	memset(&m_UdpRemoteInfo, 0, sizeof(m_UdpRemoteInfo));
}

GSSocketUDP::~GSSocketUDP(void)
{
}

BOOL GSSocketUDP::Initialize(VOID)
{
//	memset(&m_UdpRemoteInfo, 0, sizeof(m_UdpRemoteInfo));
	GSSocket::Initialize();
	
	m_ReliableUdpThreadHandle		= NULL;
	m_ReliableUdpThreadStartupEvent	= NULL;
	m_ReliableUdpThreadDestroyEvent	= NULL;
	m_ReliableUdpThreadWakeUpEvent	= NULL;
	m_ReliableUdpWriteCompleteEvent	= NULL;

	m_IsReliableUdpSending			= FALSE;

	return TRUE;

}

BOOL GSSocketUDP::Termination(VOID)
{

	GSSocket::Termination();

	m_bConnected = FALSE;

	if (m_ReliableUdpThreadHandle)
	{
		SetEvent(m_ReliableUdpThreadDestroyEvent);

		WaitForSingleObject(m_ReliableUdpThreadHandle, INFINITE);

		CloseHandle(std::exchange(m_ReliableUdpThreadHandle, nullptr));
	}

	if (m_ReliableUdpThreadDestroyEvent)
		CloseHandle(std::exchange(m_ReliableUdpThreadDestroyEvent, nullptr));

	if (m_ReliableUdpThreadStartupEvent)
		CloseHandle(std::exchange(m_ReliableUdpThreadStartupEvent, nullptr));

	if (m_ReliableUdpThreadWakeUpEvent)
		CloseHandle(std::exchange(m_ReliableUdpThreadWakeUpEvent, nullptr));

	if (m_ReliableUdpWriteCompleteEvent)
		CloseHandle(std::exchange(m_ReliableUdpWriteCompleteEvent, nullptr));

	while( !m_ReliableWriteQueue.empty() ) m_ReliableWriteQueue.pop();

	return TRUE;
}

BOOL GSSocketUDP::CreateUdpSocket(USHORT port)
{
	CThreadSync Sync(*this);

	if (m_Socket)
		return FALSE;

	SOCKADDR_IN RemoteAddressInfo;

	RemoteAddressInfo.sin_family			= AF_INET;
	RemoteAddressInfo.sin_port				= htons(port);
	RemoteAddressInfo.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);

	m_Socket = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_Socket == INVALID_SOCKET)
	{
		m_Socket = 0;
		return FALSE;
	}

	if (::bind(m_Socket, (struct sockaddr*) &RemoteAddressInfo, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		Termination();

		return FALSE;
	}

	m_BoundPort.store(port, std::memory_order_relaxed);

	// m_ReliableUdpThreadDestroyEvent
	m_ReliableUdpThreadDestroyEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (m_ReliableUdpThreadDestroyEvent == NULL)
	{
		Termination();

		return FALSE;
	}
	//

	// m_ReliableUdpThreadStartupEvent
	m_ReliableUdpThreadStartupEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (m_ReliableUdpThreadStartupEvent == NULL)
	{
		Termination();

		return FALSE;
	}
	//

	// m_ReliableUdpThreadWakeUpEvent
	m_ReliableUdpThreadWakeUpEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (m_ReliableUdpThreadWakeUpEvent == NULL)
	{
		Termination();

		return FALSE;
	}
	//

	// m_ReliableUdpWriteCompleteEvent
	m_ReliableUdpWriteCompleteEvent = CreateEvent(0, FALSE, FALSE, 0);
	if (m_ReliableUdpWriteCompleteEvent == NULL)
	{
		Termination();

		return FALSE;
	}
	//

	while( !m_ReliableWriteQueue.empty() ) m_ReliableWriteQueue.pop();

//	DWORD ReliableUdpThreadID	= 0;
//	m_ReliableUdpThreadHandle	= CreateThread(NULL, 0, GSNetwork::GSSocket::GSSocketUDP::ReliableUdpThreadCallback, this, 0, &ReliableUdpThreadID);

// 	WaitForSingleObject(m_ReliableUdpThreadStartupEvent, INFINITE);

	return TRUE;
}

BOOL GSSocketUDP::InitializeReadFromForIocp(VOID)
{
	CThreadSync sync(*this);
	if (!m_Socket || m_Socket == INVALID_SOCKET)
		return FALSE;

	auto operation = std::unique_ptr<OVERLAPPED_EX>(new (std::nothrow) OVERLAPPED_EX);
	if (!operation)
	{
		m_ReadPostFailed.fetch_add(1, std::memory_order_relaxed);
		return FALSE;
	}
	operation->IoType = IO_UDP_READ;
	operation->ObjectId = m_ClientId;
	WSABUF buffer{MAX_BUFFER_LENGTH, reinterpret_cast<char*>(m_ReceiveBuffer)};
	auto* pending = operation.release();
	m_OLP_REMAIN_COUNT_REC.fetch_add(1);
	const int result = WSARecvFrom(m_Socket, &buffer, 1, nullptr, &pending->ReceiveFlags,
		reinterpret_cast<SOCKADDR*>(&m_UdpRemoteInfo), &pending->ReceiveAddressLength,
		&pending->Overlapped, nullptr);
	const int error = result == SOCKET_ERROR ? WSAGetLastError() : 0;
	if (result == SOCKET_ERROR && error != WSA_IO_PENDING)
	{
		operation.reset(pending);
		{
			std::lock_guard<std::mutex> lock(m_ReceiveCompletionLock);
			m_OLP_REMAIN_COUNT_REC.fetch_sub(1);
			m_ReceivesDrained.notify_all();
		}
		const auto failures = m_ReadPostFailed.fetch_add(1, std::memory_order_relaxed) + 1;
		if (failures <= 4 || failures % 1024 == 0)
			SYSLOG().Write("[UDP/receive-post-failed] Port=%u ClientId=%d Error=%d Total=%llu\n",
				static_cast<unsigned>(m_BoundPort.load()), m_ClientId, error, failures);
		return FALSE;
	}

	m_ReadPosted.fetch_add(1, std::memory_order_relaxed);
	return TRUE;
}

void GSSocketUDP::CompleteIocpRead()
{
	m_LastReadCompleted.store(GetTickCount64(), std::memory_order_relaxed);
	m_ReadCompleted.fetch_add(1, std::memory_order_relaxed);
	std::lock_guard<std::mutex> lock(m_ReceiveCompletionLock);
	if (m_OLP_REMAIN_COUNT_REC.fetch_sub(1) == 1)
		m_ReceivesDrained.notify_all();
}

void GSSocketUDP::WaitForIocpReads()
{
	std::unique_lock<std::mutex> lock(m_ReceiveCompletionLock);
	m_ReceivesDrained.wait(lock, [this] { return m_OLP_REMAIN_COUNT_REC.load() == 0; });
}

void GSSocketUDP::TryRestoreIocpRead()
{
	if (m_OLP_REMAIN_COUNT_REC.load() != 0 || m_ReadPosted.load() == 0)
		return;

	CThreadSync sync(*this);
	if (m_OLP_REMAIN_COUNT_REC.load() == 0 && InitializeReadFromForIocp())
		m_ReadRestarts.fetch_add(1, std::memory_order_relaxed);
}

void GSSocketUDP::RecordHandlerTime(ULONGLONG durationMs)
{
	if (durationMs >= 5)
		m_HandlersOver5Ms.fetch_add(1, std::memory_order_relaxed);
	auto peak = m_MaxHandlerMs.load(std::memory_order_relaxed);
	while (durationMs > peak &&
		!m_MaxHandlerMs.compare_exchange_weak(peak, durationMs, std::memory_order_relaxed))
	{
	}
}

void GSSocketUDP::LogReceiveStats()
{
	const auto lastCompleted = m_LastReadCompleted.load(std::memory_order_relaxed);
	const auto age = lastCompleted ? GetTickCount64() - lastCompleted : 0;
	SYSLOG().Write("[UDP/port] Port=%u ClientId=%d ReadPending=%d Posted=%llu Completed=%llu"
		" PostFailed=%llu Restarts=%llu ReceiveErrors=%llu InvalidDatagrams=%llu"
		" LastCompletionAgeMs=%llu SendPending=%d HandlersAtLeast5Ms=%llu MaxHandlerMs=%llu\n",
		static_cast<unsigned>(m_BoundPort.load()), m_ClientId, m_OLP_REMAIN_COUNT_REC.load(),
		m_ReadPosted.load(), m_ReadCompleted.load(), m_ReadPostFailed.load(), m_ReadRestarts.load(), ReceiveErrors.load(),
		InvalidDatagrams.load(), age, m_OLP_REMAIN_COUNT_SND.load(), m_HandlersOver5Ms.load(), m_MaxHandlerMs.load());
}

BOOL GSSocketUDP::ReadFromForEventSelect(LPSTR remoteAddress, USHORT& remotePort, BYTE* data, DWORD& dataLength)
{
	CThreadSync sync(*this);
	dataLength = 0;
	if (!m_Socket || m_Socket == INVALID_SOCKET || !remoteAddress || !data)
		return FALSE;

	int addressLength = sizeof(m_UdpRemoteInfo);
	const int bytes = recvfrom(m_Socket, reinterpret_cast<char*>(data), MAX_BUFFER_LENGTH, 0,
		reinterpret_cast<SOCKADDR*>(&m_UdpRemoteInfo), &addressLength);
	if (bytes <= 0)
		return FALSE;

	strcpy(remoteAddress, inet_ntoa(m_UdpRemoteInfo.sin_addr));
	remotePort = ntohs(m_UdpRemoteInfo.sin_port);
	USHORT ack = 0;
	if (bytes >= sizeof(ack))
		memcpy(&ack, data, sizeof(ack));
	if (bytes == sizeof(ack) && ack == 9999)
	{
		SetEvent(m_ReliableUdpWriteCompleteEvent);
		return FALSE;
	}
	ack = 9999;
	WriteTo2(remoteAddress, remotePort, reinterpret_cast<BYTE*>(&ack), sizeof(ack));
	dataLength = static_cast<DWORD>(bytes);
	return TRUE;
}

VOID GSSocketUDP::SetUdpWriteCompleteEvent(VOID)
{
	SetEvent(m_ReliableUdpWriteCompleteEvent);
}


BOOL	GSSocketUDP::ReadFromForIocp(LPSTR& remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength)
{
	CThreadSync Sync(*this);

	if (!m_Socket)
		return FALSE;

	if (!data || dataLength <= 0)
		return FALSE;

	if (dataLength > MAX_BUFFER_LENGTH)
		return FALSE;

	memcpy(data, m_ReceiveBuffer, dataLength);

	remoteAddress =  inet_ntoa(m_UdpRemoteInfo.sin_addr);
	remotePort	= ntohs(m_UdpRemoteInfo.sin_port);

/*	USHORT Ack = 0;
	memcpy(&Ack, m_Buffer, sizeof(USHORT));

	if (Ack == 9999)
	{
		SetEvent(m_ReliableUdpWriteCompleteEvent);

		return FALSE;
	}
	else
	{
		Ack = 9999;
		WriteTo2(remoteAddress, remotePort, (BYTE*) &Ack, sizeof(USHORT));
	}*/

	return TRUE;
}


BOOL GSSocketUDP::GetRemoteAddressAfterAccept(LPTSTR remoteAddress, USHORT &remotePort)
{
	CThreadSync Sync(*this);

	if (!remoteAddress)
		return FALSE;

	sockaddr_in		*Local			= NULL;
	INT				LocalLength		= 0;

	sockaddr_in		*Remote			= NULL;
	INT				RemoteLength	= 0;

	GetAcceptExSockaddrs(m_ReceiveBuffer,
		0, 
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16,
		(sockaddr **) &Local,
		&LocalLength,
		(sockaddr **) &Remote,
		&RemoteLength);

	_tcscpy(remoteAddress, (LPTSTR)(inet_ntoa(Remote->sin_addr)));
	
	remotePort = ntohs(Remote->sin_port);

	return TRUE;
}

BOOL			GSSocketUDP::WriteTo(LPCSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength)
{
	CThreadSync Sync(*this);

	if (!m_Socket)
		return FALSE;

	if (!remoteAddress || remotePort <= 0 || !data || dataLength <=0)
		return FALSE;


	std::shared_ptr<WRITE_PACKET_INFO> pWriteData(new WRITE_PACKET_INFO);

	CopyMemory(pWriteData->Data,data,sizeof(pWriteData->Data));
	pWriteData->DataLength = dataLength;
	pWriteData->Object = this;
	
	m_ReliableWriteQueue.push(pWriteData); 

	if (!m_IsReliableUdpSending)
	{
		m_IsReliableUdpSending = TRUE;
		SetEvent(m_ReliableUdpThreadWakeUpEvent);
	}

	return TRUE;

}
BOOL			GSSocketUDP::WriteTo2(LPSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength)
{
	if (!remoteAddress || !remotePort || !data || !dataLength)
		return FALSE;

	SOCKADDR_IN destination{};
	destination.sin_family = AF_INET;
	destination.sin_addr.S_un.S_addr = inet_addr(remoteAddress);
	destination.sin_port = htons(remotePort);

	std::unique_ptr<BYTE[]> copy(new BYTE[dataLength]);
	memcpy(copy.get(), data, dataLength);
	return SubmitWrite(std::move(copy), dataLength, &destination);
}

VOID GSSocketUDP::ReliableUdpThreadCallback(VOID)
{
	DWORD				EventID					= 0;
	HANDLE				ThreadEvents[2]			= {m_ReliableUdpThreadDestroyEvent, m_ReliableUdpThreadWakeUpEvent};

	CHAR				RemoteAddress[32]		= {0,};
	USHORT				RemotePort				= 0;
	BYTE				Data[MAX_BUFFER_LENGTH]	= {0,};
	DWORD				DataLength				= 0;
	VOID				*Object					= NULL;

	while (TRUE)
	{
		SetEvent(m_ReliableUdpThreadStartupEvent);

        EventID = WaitForMultipleObjects(2, ThreadEvents, FALSE, INFINITE);
		switch (EventID)
		{
		case WAIT_OBJECT_0:
			return;

		case WAIT_OBJECT_0 + 1:
NEXT_DATA:

			std::shared_ptr<WRITE_PACKET_INFO> pWriteData = m_ReliableWriteQueue.front();
			// Write 일 경우 Pop을 해주고
//			if (m_ReliableWriteQueue.Pop(&Object, Data, DataLength, RemoteAddress, RemotePort))
			if(pWriteData != NULL)
			{
				// 데이터가 있을 경우
				// 실제 Write를 해 주고 WaitForSingleObject를 해 준다.
				// 받았을때 그 SetEvent를 해 주면 풀린다.
RETRY:
				if (!WriteTo2(pWriteData->RemoteAddress, pWriteData->RemotePort, pWriteData->Data, pWriteData->DataLength))
				{
					m_ReliableWriteQueue.pop();
					return;
				}
			
				m_ReliableWriteQueue.pop();

				DWORD Result = WaitForSingleObject(m_ReliableUdpWriteCompleteEvent, 10);

				// 받았을 경우 다음 데이터 처리
				if (Result == WAIT_OBJECT_0)
					goto NEXT_DATA;
				else
					goto RETRY;
			}
			else
				m_IsReliableUdpSending = FALSE;

			break;
		}
	}
}


}	}	}