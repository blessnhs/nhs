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
	{	//�α��� ���� ���� ������ ��Ŷ�� ��û ������
		// �����̳� �α��� ��Ŷ�� �ƴ� ��� ó������ �ʴ´�.
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
		
		//hub�� ����

		auto hub = PROXYHUB.GetHub(Client->GetId());
		if (hub != NULL)
		{
			if (hub->GetConnected() == FALSE)
				return;

			//���������� �����ʰ� ���� �׳� ���޸� �Ѵ�.
			hub->GetTCPSocket()->RelayPacket(pBuffer->MainId, pBuffer->SubId, pBuffer->IsCompress, (BYTE*)pBuffer->m_Buffer.GetBuffer(), pBuffer->m_Buffer.GetLength(), Client->GetId());
		}


	}
	catch (int exception)
	{
		printf("handle exception %d\n", exception);
	}
}

