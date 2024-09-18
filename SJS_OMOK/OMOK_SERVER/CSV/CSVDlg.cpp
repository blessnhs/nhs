
// CSVDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "CSV.h"
#include "CSVDlg.h"
#include "afxdialogex.h"
#include "GSClientSocket.h"
#include "../GSPacket/Front.h"
#include "../GSPacket/auth.h"

#include "LOBBY.h"
#include "ROOM.h"
#include "UDPSession.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};



CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCSVDlg ��ȭ ����




CCSVDlg::CCSVDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCSVDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCSVDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCSVDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCSVDlg::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCSVDlg �޽��� ó����

BOOL CCSVDlg::OnInitDialog()
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

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.
	

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);

	m_SocketLogin.BeginTcp("127.0.0.1",8599);	

	SetTimer(0,10,NULL);
	
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

BOOL CCSVDlg::PreTranslateMessage(MSG* pMsg)
{
	return CDialog::PreTranslateMessage(pMsg); 
}

void CCSVDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	
 	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CCSVDlg::OnPaint()
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

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CCSVDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCSVDlg::OnBnClickedOk()
{

	char posbuff[10];

	sprintf(posbuff	,"\"%d\"",0);

	CString ID,PWD;

	GetDlgItemText(IDC_LOGIN_EDIT1,ID);
	GetDlgItemText(IDC_LOGIN_EDIT2,PWD);

	Json::Value root2;
	root2["MPID"]   = ID_AUTH_LOGIN_REQ;

	PKT_AUTHENCATION_RES SendData;

	wstring id = ID;
	wstring pwd= PWD;

	std::string _Id;
	std::string _Passwd;

	_Id.assign(id.begin(),id.end());
	_Passwd.assign(pwd.begin(),pwd.end());

	root2["Id"] = _Id.c_str();
	root2["Passwd"]	 = _Passwd.c_str();

	Json::FastWriter writer;
	std::string outputConfig = writer.write( root2 );

	m_SocketLogin.WritePacket(ID_AUTH_LOGIN_REQ,0,(BYTE*)outputConfig.c_str(),outputConfig.size());

	
	
	{

	Json::Value		root2;	
	Json::Value		value;	
	Json::Value		array;	

	std::string Data = "{\"room\":[{\"Index\":5,\"Name\":\"TEST\"}]}";

	Json::Reader	reader;	
	bool bParse = reader.parse((char *)Data.c_str(), root2);	
	if(!bParse) {	
		printf("Json String Parse Error...%s\r\n",Data.c_str());	
		return;	
	}	


	bool d = root2.isArray();

	int asize = root2["room"].size();

	const Json::Value& books = root2["room"];
	for (Json::ValueConstIterator it = books.begin(); it != books.end(); ++it)
	{
		const Json::Value& book = *it;

		int Index = book["Index"].asInt();
		string name = book["Name"].asString();
		// rest as before
	}


	}




/*	PKT_AUTHENCATION_REQ req;

	req.Id = ID;
	req.Passwd = PWD;

	m_SocketLogin.Send(req);*/
} 


void CCSVDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.


	CDialogEx::OnTimer(nIDEvent);
}
