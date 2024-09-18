#pragma once


// LOBBY ��ȭ �����Դϴ�.
#include "Character.h"

class LOBBY : public CDialogEx
{
	DECLARE_DYNAMIC(LOBBY)

public:
	LOBBY(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~LOBBY();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_LOBBY };

protected:

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonQuick();
	afx_msg void OnLbnSelchangeListUser();
	afx_msg void OnBnClickedButtonMake();
	afx_msg void OnBnClickedButtonEnter();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnChangeEdit1();
	BOOL PreTranslateMessage(MSG* pMsg);

	Character m_char;
	afx_msg void OnBnClickedButtonRefresh();
};
