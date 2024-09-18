/**
 *  file    GUITask.h
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI �½�ũ�� ���õ� �Լ��� ������ ��� ����
 */

#ifndef __GUITASK_H__
#define __GUITASK_H__

#include "Types.h"
#include "Window.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �½�ũ�� ������ ���� �̺�Ʈ Ÿ�� ����
#define EVENT_USER_TESTMESSAGE          0x80000001

// �ý��� ����� �½�ũ ��ũ��
// ���μ��� ������ ǥ���ϴ� ������ �ʺ�
#define SYSTEMMONITOR_PROCESSOR_WIDTH       150
// ���μ��� ������ ǥ���ϴ� ������ ���� ������ ����
#define SYSTEMMONITOR_PROCESSOR_MARGIN      20
// ���μ��� ������ ǥ���ϴ� ������ ����
#define SYSTEMMONITOR_PROCESSOR_HEIGHT      150
// �ý��� ����� �������� ����
#define SYSTEMMONITOR_WINDOW_HEIGHT         310
// �޸� ������ ǥ���ϴ� ������ ����
#define SYSTEMMONITOR_MEMORY_HEIGHT         100
// ������ ����
#define SYSTEMMONITOR_BAR_COLOR             RGB( 55, 215, 47 )

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
// �⺻ GUI �½�ũ�� Hello World GUI �½�ũ
void WindowManagerGUITask( void );
void HelloWorldGUITask( void );

// �ý��� ����� �½�ũ �Լ�
void SystemMonitorTask( void );
static void DrawProcessorInformation( QWORD qwWindowID, int iX, int iY, 
        BYTE bAPICID );
static void DrawMemoryInformation( QWORD qwWindowID, int iY, int iWindowWidth );

// GUI �ܼ� �� �½�ũ �Լ�
void GUIConsoleShellTask( void );
static void ProcessConsoleBuffer( QWORD qwWindowID );

// �̹��� ��� �½�ũ �Լ�
void ImageViewerTask( void );
static void DrawFileName( QWORD qwWindowID, RECT* pstArea, char *pcFileName, 
        int iNameLength );
static BOOL CreateImageViewerWindowAndExecute( QWORD qwMainWindowID, 
        const char* pcFileName );

#endif /*__GUITASK_H__*/
