#pragma once
#include "GSProactor.h"
#include "SingleTonHolder.h"

namespace GSNetwork	{	namespace GSMainProcess	{

typedef GSFrames::GSProactorImpl::GSProactorImpl GSProactorImpl;

/// @brief 서버 로직 및 DB 작업을 Proactor의 작업 큐로 처리하는 서비스입니다.
class GSMainProcess : public GSFrames::GSProactor::GSProactor, public Singleton<GSMainProcess>
{
public:

	/// @brief 기본 또는 지정된 Proactor 구현을 사용하는 메인 처리 객체를 생성합니다.
	GSMainProcess(GSProactorImpl *ProactorImpl);
	/// @brief 기본 또는 지정된 Proactor 구현을 사용하는 메인 처리 객체를 생성합니다.
	GSMainProcess();
	/// @brief 메인 처리 객체를 파괴합니다.
	~GSMainProcess(void);

	/// @brief 메인 처리 이벤트 콜백입니다. 현재 기본 구현은 비어 있습니다.
	virtual void OnEvt(LPVOID Arg);
};

}	}

#define  MAINPROC Singleton<GSNetwork::GSMainProcess::GSMainProcess>::Instance()

#define PROC_REG_QUERY_JOB(REQUEST,_MID,OBJ,TARG,ARG)\
	boost::shared_ptr<TARG<ARG>>		PLAYER_MSG = ALLOCATOR.Create<TARG<ARG>>();\
	PLAYER_MSG->pSession	= OBJ;\
	PLAYER_MSG->pRequst		= REQUEST;\
	PLAYER_MSG->Type		= OBJ->GetMyDBTP(MSG_TYPE_DB_1);\
	PLAYER_MSG->SubType		= ONQUERY;\
	PLAYER_MSG->MsgId		= _MID;\
	MAINPROC.RegisterCommand(PLAYER_MSG);


#define PROC_REG_QUERY_RES_JOB(RES,OBJ,RESULT,TARG,ARG)\
	boost::shared_ptr<TARG<ARG>>	    pPlayerMsg = ALLOCATOR.Create<TARG<ARG>>();\
	pPlayerMsg->Type	= OBJ->GetMyTP();\
	pPlayerMsg->SubType = ONQUERY_RES;\
	pPlayerMsg->pResponse = RES;\
	pPlayerMsg->pSession  = OBJ;\
	pPlayerMsg->Result    = RESULT;\
	MAINPROC.RegisterCommand(pPlayerMsg);

#define PROC_REG_CLOSE_JOB(OBJ,SERVER)\
	MSG_PLAYER_CLOSE_PTR pPlayerClose = ALLOCATOR.Create<MSG_PLAYER_CLOSE>();\
	pPlayerClose->pClient	=	OBJ;\
	pPlayerClose->pListen	=	SERVER->GetTcpListen();\
	pPlayerClose->Type	    =   OBJ->GetMyTP();\
	pPlayerClose->SubType   =   ONCLOSE;\
	MAINPROC.RegisterCommand(pPlayerClose);

