// LOBBY.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "CSV.h"
#include "LOBBY.h"
#include "afxdialogex.h"
#include "../GSPacket/Front.h"
#include "GSClientSocket.h"

#include "lib_json/json/reader.h"
#include "lib_json/json/writer.h"

#include "GSSerialize.h"

extern GSClientSocket m_SocketLobby;
// LOBBY ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(LOBBY, CDialogEx)

LOBBY::LOBBY(CWnd* pParent /*=NULL*/)
	: CDialogEx(LOBBY::IDD, pParent)
{

}

LOBBY::~LOBBY()
{
}

void LOBBY::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LOBBY, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_QUICK, &LOBBY::OnBnClickedButtonQuick)
	ON_LBN_SELCHANGE(IDC_LIST_USER, &LOBBY::OnLbnSelchangeListUser)
	ON_BN_CLICKED(IDC_BUTTON_MAKE, &LOBBY::OnBnClickedButtonMake)
	ON_BN_CLICKED(IDC_BUTTON_ENTER, &LOBBY::OnBnClickedButtonEnter)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_EN_CHANGE(IDC_EDIT1, &LOBBY::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &LOBBY::OnBnClickedButtonRefresh)
END_MESSAGE_MAP()


// LOBBY �޽��� ó�����Դϴ�.


void LOBBY::OnBnClickedButtonQuick()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}

BOOL LOBBY::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	//	p_login = new login;

	//	p_login->ShowWindow(SM_SHOWSOUNDS);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetTimer(0,10,NULL);
	
	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void LOBBY::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

	}
	else
	{
		CDialogEx::OnPaint();
	}


}


BOOL LOBBY::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		switch( pMsg->wParam )
		{
		case VK_RETURN:
		case VK_ESCAPE:
			{
				CString msg;
				GetDlgItem(IDC_EDIT1)->GetWindowText(msg);

				CF_PKT_LOBBY_CHAT_NTY snd;

				//WideCharToMultiByte(CP_ACP,0,msg,-1,snd.Msg,sizeof(snd.Msg),0,0);
				snd.Msg = msg.GetBuffer(msg.GetLength());

			//	m_SocketLobby.Send(snd);

				DECLARE_JSON_WRITER
				ADD_JSON_WSTR_MEMBER("Msg",snd.Msg)

				Json::FastWriter writer;	
				std::string outputConfig = writer.write( root2 );	
				m_SocketLobby.WritePacket(ID_CF_PKT_LOBBY_CHAT_NTY,0,(BYTE*)outputConfig.c_str(),outputConfig.size()); 


				GetDlgItem(IDC_EDIT1)->SetWindowText(_T(""));
				return CWnd::PreTranslateMessage(pMsg);
			}
			break;
		}
	}

	return CDialog::PreTranslateMessage(pMsg); 
}

void LOBBY::OnLbnSelchangeListUser()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}


void LOBBY::OnBnClickedButtonMake()
{
	DECLARE_JSON_WRITER
	ADD_JSON_MEMBER("ChannelId",0)

	wstring Name = _T("TEST");

	ADD_JSON_WSTR_MEMBER("Name",Name)
	ADD_JSON_MEMBER("ChannelId",0)
	ADD_JSON_MEMBER("MaxCount",20)
	ADD_JSON_MEMBER("Type",0)

	GSClientSocket *pSocket = &m_SocketLobby;

	END_JSON_MEMBER(pSocket,ID_CF_PKT_CREATE_ROOM_REQ)

}

void LOBBY::OnBnClickedButtonEnter()
{
	CListBox *pList=(CListBox*)GetDlgItem(IDC_LIST_ROOM); 
	if (pList) 
	{
		int pos = pList->GetCurSel();
		if(pos != -1)
		{
			CString data;
			pList->GetText(pos,data);

			int idx_pos = data.Find('_');
			if(idx_pos != -1)
			{
				CString Index = data.Left(idx_pos);

				char cRoomNumber[256];
				WideCharToMultiByte( CP_ACP, 0,Index.GetBuffer(), -1, cRoomNumber, 256, NULL, NULL );

				int room_number = atoi(cRoomNumber);

				DECLARE_JSON_WRITER
				ADD_JSON_MEMBER("RoomNumber",room_number)

				Json::FastWriter writer;	
				std::string outputConfig = writer.write( root2 );	
				m_SocketLobby.WritePacket(ID_CF_PKT_ENTER_ROOM_REQ,0,(BYTE*)outputConfig.c_str(),outputConfig.size()); 



			}
		}
	}
}


void LOBBY::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	m_SocketLobby.Proc();
	
	CDialogEx::OnTimer(nIDEvent);
}

void LOBBY::OnEnChangeEdit1()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}


void LOBBY::OnBnClickedButtonRefresh()
{
	DECLARE_JSON_WRITER
	Json::FastWriter writer;	
	std::string outputConfig = writer.write( root2 );	
	m_SocketLobby.WritePacket(ID_CF_PKT_ROOM_LIST_REQ,0,(BYTE*)outputConfig.c_str(),outputConfig.size()); 
}
