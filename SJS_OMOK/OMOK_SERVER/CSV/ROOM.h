#pragma once


// ROOM ��ȭ �����Դϴ�.

class ROOM : public CDialogEx
{
	DECLARE_DYNAMIC(ROOM)

public:
	ROOM(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~ROOM();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ROOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	afx_msg void OnPaint();

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLeave();

	DWORD RoomNumber;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonStun2();
	
	afx_msg void OnBnClickedButtonStun3();
	afx_msg void OnLbnSelchangeListChat();
};
