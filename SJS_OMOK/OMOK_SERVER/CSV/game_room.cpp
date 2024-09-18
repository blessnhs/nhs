// game_room.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "CSV.h"
#include "game_room.h"
#include "afxdialogex.h"
#include "GSClientSocket.h"
#include "../GSPacket/relay.h"

#include "lib_json/json/reader.h"
#include "lib_json/json/writer.h"

#include "GSSerialize.h"
#include "UDPSession.h"
#include "vector"


std::vector<UDP_Client*> udp_client;

// game_room ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(game_room, CDialogEx)

game_room::game_room(CWnd* pParent /*=NULL*/)
	: CDialogEx(game_room::IDD, pParent)
{

}

game_room::~game_room()
{
}

void game_room::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(game_room, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_STUN, &game_room::OnBnClickedButtonStun)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STUN2, &game_room::OnBnClickedButtonStun2)
END_MESSAGE_MAP()


// game_room �޽��� ó�����Դϴ�.


void game_room::OnBnClickedButtonStun()
{
	DECLARE_JSON_WRITER
	ADD_JSON_MEMBER("Id",1)

	Json::FastWriter writer;	
	std::string outputConfig = writer.write( root2 );	
	m_SocketRelay.WritePacket(ID_CR_ENTER_ROOM_REQ,0,(BYTE*)outputConfig.c_str(),outputConfig.size()); 
}


void game_room::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	CDialogEx::OnTimer(nIDEvent);
}


void game_room::OnBnClickedButtonStun2()
{
	UDP_Client *pClient = new UDP_Client();
	udp_client.push_back( pClient );

	printf("MyIndex %d\n",MyIndex);

	pClient->Connect("127.0.0.1",pClient->MyPort,MyIndex,1);
}
