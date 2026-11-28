#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "GSPacketTCP.h"
#include "GSPacketUDP.h"
#include "IProcess.h"
#include "IResponse.h"
#include "IHandler.h"
#include "./IMessage.h"
#include "IClientSession.h"

using namespace GSNetwork;
//namespace GSNetwork	{	namespace GSClient	{

typedef GSSocket::GSPacket::GSPacketTCP::GSPacketTCP GSPacketTCP;
typedef GSSocket::GSPacket::GSPacketUDP::GSPacketUDP GSPacketUDP;

//패킷 필터링 관련 정보 구조체
/// @brief 패킷 종류별 남용 검사에 쓰이는 상태를 보관합니다.
struct stPacketContext
{
	DWORD m_dwTime;
	BOOL  m_bUse;
};

/// @brief 로그에 전달할 이벤트 정보를 보관합니다.
struct _LogContext
{
	WCHAR account[MAX_ID_LENGTH];
	bool IsSendedAuth;
	DWORD Time;
};

enum ClientType
{
	CLIENT,				//일반유저	
	SERVER_CLIENT		// 서버간에 연결
};

/// @brief 클라이언트 생성·소멸과 맵 제거 후 잔존 상태의 집계입니다.
struct GSClientLifetimeStats
{
	unsigned long long Created;
	unsigned long long Destroyed;
	unsigned long long RemovedFromMap;
	unsigned long long IdsRetiredWithPendingIo;
	int PeakLive;
	int RemovedStillLive;
};

/// @brief TCP 세션 또는 UDP 포트와 연결 상태, 패킷 처리기, 작업 라우팅 정보를 관리합니다.
class GSClient : public GSGuard<GSClient>,public IClientSession::IClientSession ,public IResponse , public IHandler<IMessagePtr>, public boost::enable_shared_from_this<GSClient>
{
public:
	/// @brief 클라이언트 생성·소멸과 제거 관련 전역 카운터를 읽습니다. 각 값은 독립적으로 집계됩니다.
	static GSClientLifetimeStats GetLifetimeStats();
	/// @brief 맵 소유권이 제거된 객체를 소멸 지연 추적 목록에 등록합니다.
	void MarkRemovedFromMap();
	/// @brief 맵 제거 후 30초 넘게 살아 있는 객체 수와 오래된 객체의 ID를 기록합니다.
	static void LogDetachedClients();


	/// @brief 연결 및 작업 상태를 기본값으로 초기화하고 생성 카운터를 갱신합니다.
	GSClient(void);
	/// @brief 소켓을 정리하고 ID 재사용 가능 여부 및 소멸 카운터를 갱신합니다.
	~GSClient(void);

	/// @brief 전송 방식과 클라이언트 유형을 설정하고 해당 소켓 객체를 준비합니다.
	BOOL											Create(BYTE Type,ClientType type = CLIENT);

	/// @brief 클라이언트가 보관한 연결 및 작업 상태를 정리합니다.
	VOID											Clear();

	/// @brief 현재 전송 방식에 해당하는 소켓 핸들을 반환합니다.
	SOCKET											GetSocket(VOID);

	/// @brief TCP 패킷 소켓의 공유 참조를 반환합니다.
	boost::shared_ptr<GSPacketTCP>					GetTCPSocket();
	/// @brief UDP 패킷 소켓의 공유 참조를 반환합니다.
	boost::shared_ptr<GSPacketUDP>					GetUDPSocket();

	/// @brief 현재 전송 방식의 다음 비동기 수신을 등록합니다.
	BOOL											InitializeReadForIocp(VOID);

	/// @brief 클라이언트의 연결 상태를 원자적으로 갱신합니다.
	VOID											SetConnected(BOOL bConnected) ;
	/// @brief 현재 클라이언트 연결 상태를 읽습니다.
	BOOL											GetConnected(VOID); 

	/// @brief 현재 전송 방식의 송신 완료 후처리를 호출합니다.
	BOOL											WriteComplete(VOID);

	/// @brief 현재 패킷 처리기의 공유 참조를 안전하게 읽습니다.
	boost::shared_ptr<IProcess<int>>				GetProcess();
	/// @brief 패킷 처리기의 공유 참조를 교체합니다.
	void											SetProcess(boost::shared_ptr<IProcess<int>> Process);

	/// @brief 템플릿으로 지정한 처리기를 생성하여 이 클라이언트에 연결합니다.
	template<class U>
	VOID   										    SetHandler()
	{
		
		boost::shared_ptr<U> Handler(new U);
		SetProcess(Handler);
	}

	/// @brief 응답 콜백의 기본 구현입니다.
	virtual VOID									OnResponse(LPVOID Data);
	/// @brief 메시지 이벤트 콜백의 기본 구현입니다.
	virtual void									OnEvt(IMessagePtr Arg);

	/// @brief 유휴 연결 점검에 쓰이는 마지막 활동 시각을 설정합니다.
	void											SetAliveTime(DWORD Time);
	/// @brief 마지막 활동 시각을 반환합니다.
	DWORD											GetAliveTime();

	/// @brief 진행 중인 DB 작업 집계를 증가시킵니다.
	void											IncDBStamp();
	/// @brief 완료된 DB 작업을 집계에서 차감합니다.
	void											DecDBStamp();
	/// @brief 현재 DB 작업 집계를 반환합니다.
	DWORD											GetDBStamp();

	/// @brief 종료 예정 상태를 설정합니다.
	void											SetWillBeTerminated(bool _terminate);
	/// @brief 종료 예정 상태를 반환합니다.
	BOOL											GetWillBeTerminated();

	/// @brief 현재 클라이언트의 패킷 처리 작업을 로직 큐에 등록합니다.
	VOID											TakeMsg(boost::shared_ptr<GSClient> client);


	/// @brief 이름으로 지정한 작업의 완료 상태를 확인합니다.
	bool											GetCompleteJob(const std::wstring _str);
	/// @brief 이름으로 지정한 작업의 완료 상태를 갱신합니다.
	void											SetCompleteJob(const std::wstring _str,bool Value);
	/// @brief DB 작업 집계에 기록된 시각을 반환합니다.
	DWORD											GetDBStampTime();

public:	

	/// @brief 송신 이벤트를 처리하는 가상 콜백입니다. 기본 구현은 비어 있습니다.
	virtual void									OnSend(WORD MainId,WORD SubId,char *Data,WORD Length);
	/// @brief 완료된 수신을 파싱하고 TCP는 로직 큐에 넘기며 UDP는 해당 포트의 처리기로 전달합니다.
	virtual void									OnRecv(DWORD Length, boost::shared_ptr<GSClient> client);
	/// @brief 연결 종료 요청을 받아 클라이언트의 종료 처리 흐름을 시작합니다.
	virtual void									OnDisconnect(boost::shared_ptr<GSClient> client, bool isForce = false);
	/// @brief 연결 승인 뒤 클라이언트의 초기화와 접속 알림을 처리합니다.
	virtual void									OnConnect(boost::shared_ptr<GSClient> client);

	/// @brief 준비된 패킷을 큐에서 꺼내 현재 처리기의 전송 방식별 콜백을 호출합니다.
	VOID											ProcPacket(boost::shared_ptr<GSClient> pClient);
	/// @brief 종료 작업을 등록하고 연결 상태 및 클라이언트 맵을 갱신합니다.
	VOID											ProcDisconnect(boost::shared_ptr<GSClient> pClient,bool isForce = false);

	/// @brief 클라이언트 ID로 로직 스레드 인덱스를 계산합니다.
	WORD											GetMyTP();
	/// @brief 기준 인덱스와 클라이언트 ID로 DB 스레드 인덱스를 계산합니다.
	WORD											GetMyDBTP(WORD StartIndex = MSG_TYPE_DB_1);

	/// @brief 현재 TCP 소켓 핸들을 닫아 이후 완료 처리 경로에서 연결을 정리할 수 있게 합니다.
	VOID											Close();
	/// @brief 생성 때 지정한 전송 방식인 TCP 또는 UDP를 반환합니다.
	BYTE											GetCreateType();

	/// @brief 생성 때 지정한 클라이언트 유형을 반환합니다.
	ClientType										GetClientType();

	/// @brief 클라이언트를 맵에 공개하기 전에 원래 Accept를 건 리스너를 저장합니다.
	void SetAcceptListener(boost::shared_ptr<GSPacketTCP> listener);
	/// @brief 인바운드 클라이언트가 접속한 TCP 리스너를 반환합니다.
	const boost::shared_ptr<GSPacketTCP>& GetAcceptListener() const;
	/// @brief 종료 처리 중 이 클라이언트의 Accept 슬롯을 한 번만 보충할 수 있게 합니다.
	bool ClaimAcceptReplacement();

private:
	boost::shared_ptr<GSPacketTCP> m_AcceptListener;
	std::atomic<bool> m_AcceptReplacementClaimed{false};
	GSClient* m_DetachedPrevious = nullptr;
	GSClient* m_DetachedNext = nullptr;
	ULONGLONG m_DetachedAt = 0;
	DWORD m_DetachedId = 0;



	boost::shared_ptr<IProcess<int>>				m_Process;

	BYTE											m_CreateType;			//tcp,udp
	std::atomic<BOOL>								m_bConnected;

	boost::shared_ptr<GSPacketTCP>					m_TCPSocket;
	boost::shared_ptr<GSPacketUDP>					m_UDPSocket;

	std::atomic<DWORD>											m_AliveTime{0};

	BOOL											m_WillBeTerminated;
	DWORD											m_DBStamp;
	DWORD											m_DBStampTime;

	std::map<std::wstring,stPacketContext>			m_CheckAbuseMap;
	std::map<std::wstring,bool>						m_CheckCompleteMap;

	std::atomic<INT64> 											m_PairPlayerId{0};

public:
	LPVOID											m_GSServer;

	/// @brief 연결된 상대 객체의 식별자를 읽습니다.
	INT64											GetPair();

	/// @brief 연결된 상대 객체의 식별자를 설정합니다.
	void											SetPair(INT64 Player);

	//내가 삭제될 타임 종료후 iocp 메세지가 올수 있으므로 대기 했다가 제거
	DWORD											m_DeleteTime;



	ClientType										m_ClientType;

	int												m_TryCloseCount;

public:
	SOCKET											m_Socket;

};

#define GSCLIENT  GSClient

#define GSCLIENT_PTR  boost::shared_ptr<GSCLIENT>


//}	}