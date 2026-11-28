#include "StdAfx.h"

#include "FrontProcess.h"
#include "Base64.h"

using namespace google;

FrontProcess::FrontProcess(void)
{
}


FrontProcess::~FrontProcess(void)
{
}

VOID FrontProcess::Process(boost::shared_ptr<XDATA> pBuffer,boost::shared_ptr<GSClient> Client)
{
	try
	{	//로그인 하지 않은 유저가 패킷을 요청 했을때
		// 버전이나 로그인 패킷이 아닌 경우 처리하지 않는다.
		if (pBuffer->MainId != ID_PKT_VERSION_REQ && pBuffer->MainId != ID_PKT_LOGIN_REQ)
		{
		}
#ifdef  _DEBUG
		const google::protobuf::EnumDescriptor* descriptor = PROTOCOL_descriptor();

		std::string name;
		if (descriptor->FindValueByNumber(pBuffer->MainId) != NULL)
			name = descriptor->FindValueByNumber(pBuffer->MainId)->name();

	//	if (pBuffer->MainId != ID_PKT_ROOM_LIST_REQ && pBuffer->MainId != ID_PKT_PRAY_MESSAGE_REQ)
	//		BLOG("%s MainProtocol %s Length %d\n", __FUNCTION__, name.c_str(), pBuffer->Length);
#endif //  _DEBUG
		
		//hub로 전송

		auto hub = PROXYHUB.GetHub(Client->GetId());
		if (hub != NULL)
		{
			if (hub->GetConnected() == FALSE)
				return;

			//압축해제를 하지않고 허브로 그냥 전달만 한다.
			hub->GetTCPSocket()->RelayPacket(pBuffer->MainId, pBuffer->SubId, pBuffer->IsCompress, (BYTE*)pBuffer->m_Buffer.GetBuffer(), pBuffer->m_Buffer.GetLength(), Client->GetId());
		}


	}
	catch (int error)
	{
		SYSLOG().Write("[Front/client packet] %s: integer exception=%d client=%lu protocol=%u\n",
			__FUNCTION__, error, Client ? Client->GetId() : 0UL,
			pBuffer ? static_cast<unsigned>(pBuffer->MainId) : 0u);
	}
	catch (const std::exception&)
	{
		SYSLOG().Write("[Front/client packet] %s: standard exception client=%lu protocol=%u\n",
			__FUNCTION__, Client ? Client->GetId() : 0UL,
			pBuffer ? static_cast<unsigned>(pBuffer->MainId) : 0u);
		throw;
	}
	catch (...)
	{
		SYSLOG().Write("[Front/client packet] %s: unknown exception client=%lu protocol=%u\n",
			__FUNCTION__, Client ? Client->GetId() : 0UL,
			pBuffer ? static_cast<unsigned>(pBuffer->MainId) : 0u);
		throw;
	}
}

