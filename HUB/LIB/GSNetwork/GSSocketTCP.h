#pragma once

namespace GSNetwork	{ namespace GSSocket	{	namespace GSSocketTCP	{

/// @brief TCP 연결의 소켓 생성, IOCP 수신 등록, 수신 데이터 복사 및 송신을 담당합니다.
class GSSocketTCP : public GSSocket
{
public:
	/// @brief TCP 소켓 객체를 생성합니다.
	GSSocketTCP(void);
	/// @brief TCP 소켓 객체를 파괴합니다.
	virtual ~GSSocketTCP(void);

	/// @brief TCP 소켓의 공통 상태를 초기화합니다.
	BOOL Initialize(VOID);
	/// @brief 기반 소켓의 종료 처리를 호출합니다.
	BOOL Termination(VOID);
	
	/// @brief 완료된 TCP 수신 데이터를 패킷 버퍼의 남은 데이터 뒤로 복사합니다.
	int  ReadForIocp(BYTE * PacketBuffer, DWORD DataLength,DWORD RemainLength, DWORD MaxPacketBufferSize);
	/// @brief 이벤트 기반 호출 경로에서 TCP 데이터를 읽고 바이트 수를 전달합니다.
	BOOL ReadForEventSelect(BYTE *Data, DWORD &DataLength);
	/// @brief 호출자가 제공한 TCP 송신 데이터를 공통 송신 경로로 전달합니다.
	BOOL Write(BYTE *Data, DWORD DataLength);
	/// @brief 호출자가 제공한 TCP 송신 데이터를 공통 송신 경로로 전달합니다.
	BOOL Write(boost::shared_ptr<WRITE_PACKET_INFO> Packet);


	/// @brief 중첩 I/O를 지원하는 TCP 소켓을 생성합니다.
	BOOL CreateTcpSocket(VOID);
	/// @brief TCP 소켓에 다음 WSARecv를 등록하고 미완료 수신을 집계합니다.
	BOOL InitializeReadForIocp(VOID);
	/// @brief 연결된 원격 TCP 주소와 포트를 조회합니다.
	BOOL GetPeerInfo(LPSTR szAddr, unsigned short &usPort);

	/// @brief TCP 소켓의 연결 상태 플래그를 설정합니다.
	VOID SetConnected(BOOL bConnected);
	/// @brief TCP 소켓의 연결 상태 플래그를 반환합니다.
	BOOL GetConnected(VOID);

protected:
	/// @brief 소유권을 넘겨받은 TCP 송신 버퍼를 공통 송신 경로에 등록합니다.
	BOOL WriteOwned(std::unique_ptr<BYTE[]> data, DWORD length);
};

}	}	}