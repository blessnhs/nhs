#ifndef __APPLICATIONPANELTASK_H__
#define __APPLICATIONPANELTASK_H__

#include "Types.h"
#include "Window.h"
#include "Font.h"


////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���ø����̼� �г� �������� ����
#define APPLICATIONPANEL_HEIGHT             31
// ���ø����̼� �г� �������� ����
#define APPLICATIONPANEL_TITLE              "SYS_APPLICATIONPANNEL"
// ���ø����̼� �г� �����쿡 ǥ���� �ð��� �ʺ�
// 09:00 AM ���·� ǥ���ϹǷ� 8 * ��Ʈ�� �ʺ�� ���
#define APPLICATIONPANEL_CLOCKWIDTH         ( 8 * FONT_ENGLISHWIDTH )

// ���ø����̼� ����Ʈ �����쿡 ��Ÿ�� �������� ����
#define APPLICATIONPANEL_LISTITEMHEIGHT     ( FONT_ENGLISHHEIGHT + 4 )
// ���ø����̼� ����Ʈ �������� ����
#define APPLICATIONPANEL_LISTTITLE          "SYS_APPLICATIONLIST"

// ���ø����̼� �гο��� ����ϴ� ����
#define APPLICATIONPANEL_COLOR_OUTERLINE     RGB( 180, 180, 180 )
#define APPLICATIONPANEL_COLOR_MIDDLELINE    RGB( 180, 180, 180 )
#define APPLICATIONPANEL_COLOR_INNERLINE     RGB( 180, 180, 180 )
#define APPLICATIONPANEL_COLOR_BACKGROUND    RGB( 0, 0, 0 )
#define APPLICATIONPANEL_COLOR_ACTIVE        RGB( 180, 180, 180 )

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ���ø����̼� �г��� ����ϴ� ������ �����ϴ� �ڷᱸ��
typedef struct kApplicationPanelDataStruct
{
    // ���ø����̼� �г� �������� ID
    QWORD qwApplicationPanelID;
    
    // ���ø����̼� ����Ʈ �������� ID
    QWORD qwApplicationListID;
    
    // ���ø����̼� �г��� ��ư ��ġ
    RECT stButtonArea;
    
    // ���ø����̼� ����Ʈ �������� �ʺ�
    int iApplicationListWidth;
    
    // ���ø����̼� ����Ʈ �����쿡�� ������ ���콺�� ��ġ�� �������� �ε���
    int iPreviousMouseOverIndex;

    // ���ø����̼� ����Ʈ �����찡 ȭ�鿡 ǥ�õǾ����� ����
    BOOL bApplicationWindowVisible;    
} APPLICATIONPANELDATA;

// GUI �½�ũ�� ������ �����ϴ� �ڷᱸ��
typedef struct kApplicationEntryStruct
{   
    // GUI �½�ũ�� �̸�
    char* pcApplicationName;

    // GUI �½�ũ�� ��Ʈ�� ����Ʈ   
    void* pvEntryPoint;
} APPLICATIONENTRY;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void ApplicationPanelGUITask( void );

static void DrawClockInApplicationPanel( QWORD qwApplicationPanelID );
static BOOL CreateApplicationPanelWindow( void );
static void DrawDigitalClock( QWORD qwApplicationPanelID );
static BOOL CreateApplicationListWindow( void );
static void DrawApplicationListItem( int iIndex, BOOL bSelected );
static BOOL ProcessApplicationPanelWindowEvent( void );
static BOOL ProcessApplicationListWindowEvent( void );
static int GetMouseOverItemIndex( int iMouseY );
RECT GetPannelButtonRect();
#endif /*__APPLICATIONPANELTASK_H__*/
