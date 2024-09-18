#pragma once

////////////////////////////////////////////////////
// Designed for T.C.G by Timothy.
////////////////////////////////////////////////////
//v.1.0

#ifdef _WIN32
	#pragma pack( push ,1 )
#else
	#pragma pack(1)
#endif


#define _PROTOCOL_VERSION 1004

// packet result error codes.
enum ePACKET_RESULT
{
	_ERR_NONE = 0,		// Error is not occured.
	_ERR_NOTREGISTERED,	// Not registered User.
	_ERR_PWD,			// Wrong Password.
	_ERR_NETWORK,		// User socket error occured.
	_ERR_PROTOCOL,		// denied for invalid protocol version
	_ERR_OCCUR,			// �����߻�
	_ERR_DISABLE,		// �Ұ���
	_ERR_LOGINED,		// �α��ν� �̹� �α��� �Ǿ�����
	_ERR_WRONG_CHAR_NAME,	//�߸��� ĳ�����̸�
	_ERR_TOO_MANY_USERS		//������ �̹� Ǯ�Դϴ�.
};

// common packet data
typedef struct PACKET_BASE
{
	unsigned short	MPID;	// Main Packet ID
	unsigned short	SPID;	// Sub Packet ID
	unsigned short	Len;	// Packet Size
	unsigned short	FROM;	// Purpose of Packet 
} _PKT_BASE;

enum eLB_MAIN_PACKET_ID
{
	_GS_PKT_LOGIN = 1,					/// ���Ӽ��� �α���
	_GS_PKT_LOGIN_RE,					/// ���Ӽ��� �α��� ����
	_GS_PKT_LOGOUT,						/// ���Ӽ��� �α׾ƿ�
	_GS_PKT_UPDATE_DATA,
	_GS_PKT_UPDATE_DATA_RE,
	_GS_PKT_REQ_DATA,
	_GS_PKT_REQ_DATA_RE,

	_GS_PKT_MAX,						// �׻� �������� �д� �� ������ ���� ��Ŷ�� ���� �� ��Ŷ�� ��ȿ���� �����Ѵ�.
};

//Client->Server
struct LS_PKT_LOGIN : PACKET_BASE			/// �α��� ��û
{
	char	szUserID[_MAX_USER_ID_SIZE];	/// ���̵�
	char	szPwd[_MAX_PWD_SIZE];			/// �н�����
	LS_PKT_LOGIN() {
		memset(this, 0x00, sizeof(LS_PKT_LOGIN));
		MPID = _GS_PKT_LOGIN;
		Len = sizeof(LS_PKT_LOGIN);
	};
};

//Server->Client
struct LS_PKT_LOGIN_RE : PACKET_BASE				///�α��� ��û ���
{
	WORD		wResult;							/*_ERR_NONE = 0,		 // Error is not occured.
													 _ERR_NOTREGISTERED,	 // Not registered User.
													 _ERR_PWD,				 // Wrong Password.
													 _ERR_NETWORK,			 // User socket error occured.
													 _ERR_PROTOCOL,			 // denied for invalid protocol version
													 _ERR_OCCUR,			 // �����߻�
													 _ERR_DISABLE,			 // �Ұ���
													 _ERR_LOGINED,			 // �α��ν� �̹� �α��� �Ǿ�����
													 _ERR_WRONG_CHAR_NAME	 //�߸��� ĳ�����̸�"
													*/
												
	LS_PKT_LOGIN_RE() {
		memset(this, 0x00, sizeof(LS_PKT_LOGIN_RE));
		MPID = _GS_PKT_LOGIN_RE;
		Len = sizeof(LS_PKT_LOGIN_RE);
	};
};


struct GS_PKT_UPDATE_DATA : PACKET_BASE
{
	char	szUserID[_MAX_USER_ID_SIZE];	/// ���̵�
	char	szPwd[_MAX_PWD_SIZE];			/// �н�����
	char	Data[512];

	GS_PKT_UPDATE_DATA()
	{
		memset(this, 0x00, sizeof(GS_PKT_UPDATE_DATA));
		MPID = _GS_PKT_UPDATE_DATA;
		Len = sizeof(GS_PKT_UPDATE_DATA);
	}
};

struct GS_PKT_UPDATE_DATA_RE : PACKET_BASE
{
	WORD Result;		//0����

	GS_PKT_UPDATE_DATA_RE()
	{
		memset(this, 0x00, sizeof(GS_PKT_UPDATE_DATA_RE));
		MPID = _GS_PKT_UPDATE_DATA_RE;
		Len = sizeof(GS_PKT_UPDATE_DATA_RE);
	}
};
struct GS_PKT_REQ_DATA : PACKET_BASE
{
	char	szUserID[_MAX_USER_ID_SIZE];	/// ���̵�
	char	szPwd[_MAX_PWD_SIZE];			/// �н�����
	
	GS_PKT_REQ_DATA()
	{
		memset(this, 0x00, sizeof(GS_PKT_REQ_DATA));
		MPID = _GS_PKT_REQ_DATA;
		Len = sizeof(GS_PKT_REQ_DATA);
	}
};

struct GS_PKT_REQ_DATA_RE : PACKET_BASE
{
	char	Data[512];
	WORD Result;		//0����

	GS_PKT_REQ_DATA_RE()
	{
		memset(this, 0x00, sizeof(GS_PKT_REQ_DATA_RE));
		MPID = _GS_PKT_REQ_DATA_RE;
		Len = sizeof(GS_PKT_REQ_DATA_RE);
	}
};


///////////////////////////////////////////////////////////////////////
#pragma pack()
