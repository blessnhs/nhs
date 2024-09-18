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

VOID FrontProcess::Process(LPVOID Data, DWORD Length, WORD MainProtocol, WORD SubProtocol, BOOL IsCompress,boost::shared_ptr<GSClient> Client)
{
	try
	{	//�α��� ���� ���� ������ ��Ŷ�� ��û ������
		// �����̳� �α��� ��Ŷ�� �ƴ� ��� ó������ �ʴ´�.
		if (MainProtocol != ID_PKT_VERSION_REQ && MainProtocol != ID_PKT_LOGIN_REQ)
		{
		}
#ifdef  _DEBUG
		const google::protobuf::EnumDescriptor* descriptor = PROTOCOL_descriptor();
		std::string name = descriptor->FindValueByNumber(MainProtocol)->name();

		if (MainProtocol != ID_PKT_ROOM_LIST_REQ && MainProtocol != ID_PKT_PRAY_MESSAGE_REQ)
			BLOG("%s MainProtocol %s Length %d\n", __FUNCTION__, name.c_str(), Length);
#endif //  _DEBUG
		
		//hub�� ����

		auto hub = PROXYHUB.GetHub(Client->GetId());
		if (hub != NULL)
		{
			if (hub->GetConnected() == FALSE)
				return;

			//���������� �����ʰ� ���� �׳� ���޸� �Ѵ�.
			hub->GetTCPSocket()->RelayPacket(MainProtocol, Client->GetId(), IsCompress,(BYTE*)Data, Length);
		}


	}
	catch (int exception)
	{
		printf("handle exception %d\n", exception);
	}
}

