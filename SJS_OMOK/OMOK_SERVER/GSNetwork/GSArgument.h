#pragma once

class GSArgument
{
public:
	GSArgument(void);
	~GSArgument(void);

	//�ִ� ���� ���� Ŭ���̾�Ʈ
	WORD m_MaxClient;	
	
	//tcp ���ε� ��Ʈ
	WORD m_BindPort;

	//���� ������ ī��Ʈ
	WORD m_LogicThreadCnt;

	//����
	WORD m_Version;

	//���� �˰���
	BOOL m_Naggle;

	//����̺� Ÿ�� ���� second 
	DWORD m_AliveTime;

	//udp����Ʈ ��Ʈ
	std::vector<DWORD> m_UdpPorts;

	//�ִ� db thread Count
	WORD m_DBThreadCnt;
};

