#pragma once

#include <condition_variable>
#include <mutex>


namespace GSNetwork	{ namespace GSSocket	{	namespace GSSocketUDP	{

/// @brief UDP 바인딩, 비동기 데이터그램 수신과 송신 및 포트별 I/O 진단을 관리합니다.
class GSSocketUDP :	public GSSocket
{
public:
	/// @brief UDP 소켓 객체와 원격 주소 저장소를 초기화합니다.
	GSSocketUDP(void);
	/// @brief UDP 소켓 객체를 파괴합니다.
	~GSSocketUDP(void);

	/// @brief 기반 소켓과 레거시 UDP 이벤트 상태를 초기화합니다.
	BOOL Initialize(VOID);
	/// @brief 소켓과 UDP 이벤트 핸들을 정리하고 레거시 송신 큐를 비웁니다.
	BOOL Termination(VOID);

	/// @brief UDP 소켓을 생성하고 로컬 포트에 바인딩합니다.
	BOOL CreateUdpSocket(USHORT port);

	/// @brief 공용 수신 버퍼에 다음 WSARecvFrom을 등록합니다. 이전 완료 데이터 사용을 끝낸 뒤 호출합니다.
	BOOL InitializeReadFromForIocp(VOID);
	/// @brief 수신 완료 처리 한 건을 집계하고 종료 대기자에게 완료를 알립니다.
	void CompleteIocpRead();
	/// @brief 소켓을 닫은 뒤 미완료 수신이 모두 정리될 때까지 기다립니다. IOCP 워커 밖에서 호출합니다.
	void WaitForIocpReads();
	/// @brief 포트 단위 수신, 재등록, 송신 대기 및 핸들러 지연을 기록합니다.
	void LogReceiveStats();
	/// @brief 이전에 수신을 등록한 포트에 미완료 수신이 없으면 재등록을 시도합니다.
	void TryRestoreIocpRead();
	/// @brief UDP 핸들러 지연 집계와 최대 소요 시간을 갱신합니다.
	void RecordHandlerTime(ULONGLONG durationMs);
	std::atomic<unsigned long long> InvalidDatagrams{0};
	std::atomic<unsigned long long> ReceiveErrors{0};

	/// @brief 완료된 수신 버퍼의 데이터와 원격 주소를 호출자에게 전달합니다.
	BOOL ReadFromForIocp(LPSTR& remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength);
	/// @brief 이벤트 기반 경로에서 데이터그램 한 건을 읽고 레거시 ACK를 처리합니다.
	BOOL ReadFromForEventSelect(LPSTR remoteAddress, USHORT &remotePort, BYTE *data, DWORD &dataLength);
	/// @brief 레거시 신뢰형 UDP 송신 큐에 데이터를 넣습니다. 정상 서버 송신은 WriteTo2를 사용합니다.
	BOOL WriteTo(LPCSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength);
	/// @brief 지정한 원격 IPv4 주소로 보낼 데이터를 소유 버퍼에 복사하고 송신을 등록합니다.
	BOOL WriteTo2(LPSTR remoteAddress, USHORT remotePort, BYTE *data, DWORD dataLength);
	/// @brief AcceptEx 형식의 결과 버퍼에서 원격 주소를 읽는 레거시 보조 함수입니다.
	BOOL GetRemoteAddressAfterAccept(LPTSTR remoteAddress, USHORT &remotePort);
	
	/// @brief 레거시 신뢰형 UDP 송신·재시도 루프를 실행하는 콜백입니다.
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
	
	/// @brief 레거시 신뢰형 UDP의 송신 완료 이벤트를 알립니다.
	inline VOID		SetUdpWriteCompleteEvent(VOID);
};

}	}	}
