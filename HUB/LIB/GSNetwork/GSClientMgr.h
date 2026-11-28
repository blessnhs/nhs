#pragma once

#include <condition_variable>
#include <mutex>

#include "GSClient.h"


namespace GSNetwork	{	namespace GSClientMgr	{

/// @brief 클라이언트 ID와 맵 소유권, 대기 Accept, 지연 제거 및 상태 점검을 관리합니다.
class GSClientMgr : public IHandler<IMessagePtr> ,  public GSGuard<GSClientMgr>
{
public:

	/// @brief 클라이언트 관리자 객체를 생성합니다.
	GSClientMgr();
	/// @brief 클라이언트 관리자 객체를 파괴합니다.
	~GSClientMgr(void);

	/// @brief 메시지 이벤트를 받는 기본 콜백입니다.
	virtual void OnEvt(IMessagePtr Arg) {}

	/// @brief 유휴 세션, 제거 대기열과 UDP 상태를 점검하고 진단 집계를 기록합니다.
	VOID CheckAliveTime();

	//연결되어있는 소켓 갯수
	/// @brief 맵을 조회하여 연결 상태인 소켓 수를 집계합니다.
	int  GetActiveSocketCount();

	
	/// @brief 전역 증가 카운터에서 새 클라이언트 ID를 발급합니다.
	int IncClientId();

	/// @brief 리스너의 Accept 등록을 활성화합니다.
	void StartAccepting();
	/// @brief 진행 중인 Accept 등록이 끝난 뒤 새 등록을 차단합니다.
	void StopAccepting();
	/// @brief TCP 리스너의 Accept 등록이 활성화되어 있는지 확인합니다.
	bool IsAccepting() const;
	/// @brief 완료 통지를 받은 Accept를 미완료 집계에서 차감합니다.
	void CompleteAccept();
	/// @brief 리스닝 소켓을 닫은 뒤 미완료 Accept 통지가 모두 처리될 때까지 기다립니다.
	void WaitForAccepts();
	/// @brief 요청한 수만큼 TCP Accept에 사용할 클라이언트를 준비합니다.
	BOOL NewClient(int cnt = 1);
	/// @brief 지정한 TCP 리스너에 새 Accept를 등록합니다.
	BOOL NewClient(const boost::shared_ptr<GSPacketTCP>& listener, int count);

	//서버와의 연결을 위한 클라이언트 세션 생성
	/// @brief 외부 연결에 사용할 TCP 클라이언트 객체를 준비합니다.
	GSCLIENT_PTR NewClient2();

	/// @brief 관리자 종료 훅입니다. 현재 구현에는 별도 정리 단계가 없습니다.
	VOID End();

	/// @brief ID로 현재 클라이언트를 조회하며 미등록 또는 빈 슬롯이면 빈 참조를 반환합니다.
	GSCLIENT_PTR GetClient(int id);
	/// @brief 클라이언트의 소유 서버를 설정하고 ID 슬롯에 공유 참조를 등록합니다.
	BOOL AddClient(GSCLIENT_PTR client);
	/// @brief 현재 클라이언트를 찾아 지연 제거 큐에 넣습니다.
	BOOL BookDelClient(int id);
	/// @brief ID 슬롯의 클라이언트 참조를 제거하고 분리된 객체의 생존을 추적합니다.
	BOOL DelClient(int id);
	/// @brief 맵에 남은 연결 상태의 클라이언트에 소켓 닫기를 요청합니다.
	VOID AllCloseSocket();

	concurrency::concurrent_queue<GSCLIENT_PTR>				 m_ReInsert_Queue;
	concurrency::concurrent_queue<GSCLIENT_PTR>				 m_Remove_Queue;

	//클라이어언트 맵
	concurrency::concurrent_unordered_map<int, GSCLIENT_PTR> m_Clients;

	/// @brief 새로 등록되는 클라이언트가 참조할 소유 서버를 지정합니다.
	void SetGSServer(LPVOID pServer)
	{
		m_GSServer = pServer;
	}

	/// @brief 다시 사용할 수 있는 ID를 재사용 큐에 등록합니다.
	void InsertRecycleId(int _id);
	/// @brief 재사용 조건을 충족하는 ID를 꺼내며 꺼낼 ID가 없으면 -1을 반환합니다.
	int PopRecycleId();
private:

	LPVOID m_GSServer;
	std::atomic<bool> m_Accepting{false};
	std::atomic<int> m_PendingAccepts{0};
	std::mutex m_AcceptCompletionLock;
	std::condition_variable m_AcceptsDrained;

	//재사용 id
	concurrency::concurrent_queue<int>						m_Reycle_Id_Queue;

};

}	}