#pragma once

#include "GSArgument.h"
#include "GSIocp.h"
#include "GSClientMgr.h"

namespace GSNetwork	{	namespace GSServer	{


typedef GSNetwork::GSSocket::GSPacket::GSPacketTCP::GSPacketTCP GSPacketTCP;

/// @brief TCP 리스너, UDP 포트, 클라이언트 관리자와 IOCP 완료 콜백을 연결하는 서버 기반 클래스입니다.
class GSServer : public GSIocp::GSIocp , public Singleton<GSServer>, public GSGuard<GSServer>
{
public:
	enum USER_EVNT_TYPE
	{
		ONACCEPT=0,
		ONPACKET,
		ONCLOSE,
		ONQUERY,
		ONTIME,
		ONQUERY_RES,
	};

	/// @brief Winsock과 COM 초기화 및 클라이언트 관리자 연결을 수행합니다.
	GSServer(void);
	/// @brief 서버가 사용한 Winsock 및 COM 초기화를 정리합니다.
	~GSServer(void);

	/// @brief 서버 인수로 작업 스레드와 TCP·UDP 리스너를 시작하고 상태 점검을 등록합니다.
	bool														  Create(GSArgument arg);

	/// @brief IOCP와 TCP 리스닝 소켓을 준비하고 초기 Accept를 등록합니다.
	BOOL														  BeginTCP();
	/// @brief UDP 포트를 바인딩하고 각 포트를 등록한 뒤 첫 수신을 제출합니다.
	BOOL														  BeginUDP();
	/// @brief UDP 수신 완료를 기다린 뒤 IOCP와 리스너를 정리합니다. IOCP 워커 밖에서 호출합니다.
	VOID														  End(VOID);

	/// @brief 완료된 TCP·UDP 수신을 클라이언트에 전달하고 수신 작업 집계를 갱신합니다.
	virtual VOID												  OnRead(int client_id, DWORD dataLength);
	/// @brief 송신 완료 바이트와 대상 클라이언트의 미완료 송신 집계를 갱신합니다.
	virtual VOID												  OnWrote(int client_id, DWORD dataLength);
	/// @brief 완료된 Accept를 초기화하고 접속 훅 및 첫 TCP 수신을 처리합니다.
	virtual VOID												  OnConnected(int client_id);
	/// @brief 대체 Accept를 요청하고 클라이언트의 연결 종료 콜백을 호출합니다.
	virtual VOID												  OnDisconnected(int client_id,bool isForce = false);

	/// @brief I/O 종류별 완료 집계를 처리하고 TCP 종료 또는 UDP 수신 재등록을 수행합니다.
	virtual VOID 												  OnDisconnected2(int client_id, int type);

	/// @brief TCP 리스닝 클라이언트의 공유 참조를 반환합니다.
	GSCLIENT_PTR												  GetTcpListen();
	/// @brief 클라이언트 ID로 색인된 UDP 리스너 맵에 대한 참조를 반환합니다.
	concurrency::concurrent_unordered_map<DWORD, GSCLIENT_PTR>	 &GetUDPListenPorts();
	/// @brief 서버가 소유한 클라이언트 관리자에 대한 참조를 반환합니다.
	GSClientMgr::GSClientMgr									 &GetClientMgr();

	/// @brief 현재 서버 설정에 대한 참조를 반환합니다.
	GSArgument													 &GetArgument();

	/// @brief 클라이언트 관리자에 연결 소켓 닫기를 요청합니다.
	VOID														  AllClose();

	/// @brief 현재 맵에 있는 각 클라이언트에 지정한 타입의 처리기를 생성하여 설정합니다.
	template<class U>
	VOID														  SetHandler()
	{
		for (auto& kv : m_ClientMgr.m_Clients)
		{
			GSCLIENT_PTR client = boost::atomic_load(&kv.second);	// [LOCKFIX]
			if (client == NULL)
				continue;

				boost::shared_ptr<U> Handler(new U);
				client->SetProcess(Handler);
		}
	}

	/// @brief 클라이언트 관리자에서 지정 ID를 조회합니다.
	GSCLIENT_PTR												  GetClient(DWORD _Id);

	/// @brief 애플리케이션에서 구현하는 클라이언트 연결 종료 훅입니다.
	virtual BOOL												  Disconnect(GSCLIENT_PTR object) = 0;
	/// @brief 애플리케이션에서 구현하는 접속 승인 후 초기화 훅입니다.
	virtual VOID												  Accept(GSCLIENT_PTR object) = 0;


	/// @brief 압축된 입력 데이터를 해제합니다.
	std::string decompress(const std::string& data);

	/// @brief 입력 데이터를 압축합니다.
	std::string compress(const std::string& data);



public:
	atomic<unsigned long long>		TotalRecvBytes;

	atomic<unsigned long long>		TotalSendBytes;


private:


	GSClientMgr::GSClientMgr									  m_ClientMgr;

	GSCLIENT_PTR m_PrimaryTcpListener;
	std::vector<GSCLIENT_PTR> m_TcpListeners;
	concurrency::concurrent_unordered_map<DWORD,GSCLIENT_PTR>	  m_UDPListenPorts;
	concurrency::concurrent_queue<boost::shared_ptr<GSSocket::GSSocketUDP::GSSocketUDP>>			  m_FreePorts;

protected:
	GSArgument													  m_Arguments;
	int															  m_EvtClientId;
};

}	}

#define GSSERVER GSNetwork::GSServer::GSServer