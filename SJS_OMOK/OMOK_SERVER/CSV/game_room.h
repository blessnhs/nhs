#pragma once


// game_room ��ȭ �����Դϴ�.

class game_room : public CDialogEx
{
	DECLARE_DYNAMIC(game_room)

public:
	game_room(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~game_room();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_CHAT_ROOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStun();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStun2();
};
