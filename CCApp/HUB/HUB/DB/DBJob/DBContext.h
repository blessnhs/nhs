#pragma once

class Player;

typedef GSNetwork::GSObject::GSObject GSObject;

class RequestPlayerAuth : public GSObject
{
public:
	RequestPlayerAuth(void) 
	{
	}

	~RequestPlayerAuth(void) {}

	std::string token;
	INT64 Index;
	int Result;
	int channel;

	std::string MachineId;
	std::string MachineModel;
	std::string Ip;

	DWORD ForntId;		//����Ʈ ���� ���̵�(�����)
	DWORD FrontSid;		//����Ʈ �����ȿ��� � ���� Ŭ���̾�Ʈ����
};


class RequestVerifyPurchase : public GSObject
{
public:
	RequestVerifyPurchase(void)
	{
	}

	~RequestVerifyPurchase(void) {}

	std::string token;
	

	std::string PacketId;
	std::string PurchaseId;

	boost::shared_ptr<Player> pPlayer;

	DWORD ForntId;		//����Ʈ ���� ���̵�(�����)
	DWORD FrontSid;		//����Ʈ �����ȿ��� � ���� Ŭ���̾�Ʈ����
};
