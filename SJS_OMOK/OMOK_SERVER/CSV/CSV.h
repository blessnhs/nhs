
// CSV.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.
#include "LOBBY.h"


// CCSVApp:
// �� Ŭ������ ������ ���ؼ��� CSV.cpp�� �����Ͻʽÿ�.
//

class CCSVApp : public CWinApp
{
public:
	CCSVApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();



	DECLARE_MESSAGE_MAP()
};

extern CCSVApp theApp;