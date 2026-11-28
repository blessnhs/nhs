#pragma once

#include <condition_variable>
#include <mutex>


namespace GSNetwork	{ namespace GSSocket	{	namespace GSSocketUDP	{

class GSSocketUDP :	public GSSocket
{
public:
	GSSocketUDP(void);
	~GSSocketUDP(void);

	BOOL Initialize(VOID);
	BOOL Termination(VOID);

	BOOL CreateUdpSocket(USHORT port);

	BOOL InitializeReadFromForIocp(VOID);
	void CompleteIocpRead();
	void WaitForIocpReads();
	void LogReceiveStats();
	void TryRestoreIocpRead();
	void RecordHandlerTime(ULONGLONG durationMs);
	std::atomic<unsigned long long> InvalidDatagrams{0};
	std::atomic<unsigned long long> ReceiveErrors{0};

	BOOL ReadFromForIocp(LPSTR& remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength);
	BOOL ReadFromForEventSelect(LPSTR remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength);
	BOOL WriteTo(LPCSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength);
	BOOL WriteTo2(LPSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength);
	BOOL GetRemoteAddressAfterAccept(LPTSTR remoteAddress, USHORT &remotePort);
	
	VOID ReliableUdpThreadCallback(VOID);

	SOCKADDR_IN		m_UdpRemoteInfo;

private:
	std::mutex m_ReceiveCompletionLock;
	std::condition_variable m_ReceivesDrained;
	std::atomic<USHORT> m_BoundPort{0};
	std::atomic<unsigned long long> m_ReadPosted{0};
	std::atomic<unsigned long long> m_ReadCompleted{0};
	std::atomic<unsigned long long> m_ReadPostFailed{0};
	std::atomic<ULONGLONG> m_LastReadCompleted{0};
	std::atomic<unsigned long long> m_ReadRestarts{0};
	std::atomic<unsigned long long> m_HandlersOver5Ms{0};
	std::atomic<ULONGLONG> m_MaxHandlerMs{0};
	
	HANDLE			m_ReliableUdpThreadHandle = nullptr;
	HANDLE			m_ReliableUdpThreadStartupEvent = nullptr;
	HANDLE			m_ReliableUdpThreadDestroyEvent = nullptr;
	HANDLE			m_ReliableUdpThreadWakeUpEvent = nullptr;
	HANDLE			m_ReliableUdpWriteCompleteEvent = nullptr;

	std::queue<std::shared_ptr<WRITE_PACKET_INFO>>		m_ReliableWriteQueue;					
	BOOL			m_IsReliableUdpSending;
	
	inline VOID		SetUdpWriteCompleteEvent(VOID);
};

}	}	}
