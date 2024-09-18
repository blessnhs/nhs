/**
 *  file    GUITask.c
 *  date    2009/10/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GUI �½�ũ�� ���õ� �Լ��� ������ �ҽ� ����
 */
#include "GUITask.h"
#include "Window.h"
#include "MultiProcessor.h"
#include "Font.h"
#include "Console.h"
#include "Task.h"
#include "ConsoleShell.h"
#include "JPEG.h"
#include "DynamicMemory.h"
#include "fat32/fat_defs.h"
#include "fat32/fat_filelib.h"
#include "fat32/fat_access.h"


void UpdateWindowManger(QWORD qwWindowID,int count,DIRECTORY *dirlist,RECT *stButtonAreaArray,char *path)
{
    DrawRect( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT, 800, 600, RGB( 255, 255, 255 ),  TRUE );

    int idx,StartY;
    RECT stButtonArea;

    //Display Current Path
    DrawText( qwWindowID, 20, WINDOW_TITLEBAR_HEIGHT, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),	path, strlen(path ) );

    StartY = WINDOW_TITLEBAR_HEIGHT + 50;

    int fileinfowidth = 160;
    int fileinfoheight = 33;

    //One Line Max Count
    int LineCount = 12;

    for(idx = 0;idx<count;idx++)
    {
    	int yIdx = idx % LineCount;

     	// �̺�Ʈ ������ ǥ���ϴ� ������ �׵θ��� ������ ID�� ǥ��
        DrawRect( qwWindowID,
        		10 + ((idx/LineCount) * fileinfowidth), StartY + (yIdx * fileinfoheight),
        		160 + ((idx/LineCount) * fileinfowidth), StartY + fileinfoheight + (yIdx * fileinfoheight),
				RGB( 0, 0, 0 ),  FALSE );

        SetRectangleData(10 + ((idx/LineCount) * fileinfowidth), StartY + (yIdx * fileinfoheight),
        				160 + ((idx/LineCount) * fileinfowidth), StartY + fileinfoheight + (yIdx * fileinfoheight),
						&stButtonAreaArray[idx] );

    	char vcTempBuffer[1024];

        if(dirlist[idx].is_dir == 1)
        	SPrintf( vcTempBuffer, "[%s]", dirlist[idx].filename );
        else
        	SPrintf( vcTempBuffer, "%s", dirlist[idx].filename );

        DrawText( qwWindowID, 20 + ((idx/LineCount) * fileinfowidth), StartY + 8 + (yIdx * fileinfoheight), RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),vcTempBuffer, strlen( vcTempBuffer ) );
    }
}

void WindowManagerGUITask( void )
{
    if( IsGraphicMode() == FALSE )
    {
        Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }

    QWORD qwWindowID;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;

    char pathBuffer[1024];
    MemSet(pathBuffer,0,1024);

    if( IsGraphicMode() == FALSE )
    {
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
     	Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }

    //current path
    MemCpy(pathBuffer,"/",strlen("/"));

    // �������� ũ��� ���� ����
    iWindowWidth = 800;
    iWindowHeight = 600;

    qwWindowID = CreateWindow( 100, 100,
    iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, pathBuffer );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
      	Printf( "CreateWindow Failed\n" );
        return ;
    }

    Printf( "Start\n" );

    int count = fl_directoryfilecount(pathBuffer);
    DIRECTORY *dirlist = (DIRECTORY* )fl_listdirectory(pathBuffer);;
    RECT *stButtonAreaArray = NEW(sizeof(RECT) * count);

    UpdateWindowManger(qwWindowID,count,dirlist,stButtonAreaArray,pathBuffer);

    ShowWindow( qwWindowID, TRUE );

    while( 1 )
    {
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 0 );
            continue;
        }
        
        switch( stReceivedEvent.qwType )
        {
        case EVENT_MOUSE_LBUTTONUP:            

        	pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            int idx;
            for(idx = 0;idx<count;idx++)
            {
				if( IsInRectangle( &stButtonAreaArray[idx], pstMouseEvent->stPoint.iX,
													pstMouseEvent->stPoint.iY ) == TRUE )
				{
					DrawText( qwWindowID, 20, WINDOW_TITLEBAR_HEIGHT, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
							"                                                         ",
							strlen("                                                         ") );

					DrawText( qwWindowID, 20, WINDOW_TITLEBAR_HEIGHT, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
							dirlist[idx].filename, strlen(dirlist[idx].filename ) );

					if(pathBuffer,dirlist[idx].is_dir == 0)
					{
						char exebuf[1024];
						MemSet(exebuf,0,1024);

						if(strcmp(pathBuffer,"/") == 0)
							SPrintf(exebuf,"exec %s%s",pathBuffer,dirlist[idx].filename);
						else
							SPrintf(exebuf,"exec %s/%s",pathBuffer,dirlist[idx].filename);

						ExecuteCommand(exebuf);
					}
					else
					{
						if(strcmp(dirlist[idx].filename,"..") == 0)
						{
							int count = strlen(pathBuffer);

							int i;
							for(i = count - 1;i>=0 ;i--)
							{
								if(pathBuffer[i] == '/')
								{
									if(i > 0)
										pathBuffer[i] = 0;

									break;
								}
								else
									pathBuffer[i] = 0;
							}
						}
						else
						{
							if(strcmp(pathBuffer,"/") == 0)
								SPrintf(pathBuffer,"%s%s",pathBuffer,dirlist[idx].filename);
							else
							{
								SPrintf(pathBuffer,"%s/%s",pathBuffer,dirlist[idx].filename);
							}
						}

						Printf("pathBuffer %s\n",pathBuffer);

						count = fl_directoryfilecount(pathBuffer);

						if(dirlist != 0)
							FreeMemory(dirlist);

						if(stButtonAreaArray != 0)
							FreeMemory(stButtonAreaArray);

						dirlist = (DIRECTORY* )fl_listdirectory(pathBuffer);;
						stButtonAreaArray = NEW(sizeof(RECT) * count);

						UpdateWindowManger(qwWindowID,count,dirlist,stButtonAreaArray,pathBuffer);
					}

					UpdateScreenByID( qwWindowID );

				}
			}
            break;

            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // ���⿡ Ű���� �̺�Ʈ ó�� �ڵ� �ֱ�
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // ���⿡ ������ �̺�Ʈ ó�� �ڵ� �ֱ�
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            //------------------------------------------------------------------
            // ������ �ݱ� �̺�Ʈ�̸� �����츦 �����ϰ� ������ �������� �½�ũ�� ����
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                DeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            break;
        }
    }
}

//------------------------------------------------------------------------------
//  Hello World GUI �½�ũ
//------------------------------------------------------------------------------
/**
 *  Hello World GUI �½�ũ
 */
void HelloWorldGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;
    int iY;
    char vcTempBuffer[ 50 ];
    static int s_iWindowCount = 0;
    // �̺�Ʈ Ÿ�� ���ڿ�
    char* vpcEventString[] = { 
            "Unknown",
            "MOUSE_MOVE       ",
            "MOUSE_LBUTTONDOWN",
            "MOUSE_LBUTTONUP  ",
            "MOUSE_RBUTTONDOWN",
            "MOUSE_RBUTTONUP  ",
            "MOUSE_MBUTTONDOWN",
            "MOUSE_MBUTTONUP  ",
            "WINDOW_SELECT    ",
            "WINDOW_DESELECT  ",
            "WINDOW_MOVE      ",
            "WINDOW_RESIZE    ",
            "WINDOW_CLOSE     ",            
            "KEY_DOWN         ",
            "KEY_UP           " };
    RECT stButtonArea;
    QWORD qwFindWindowID;
    EVENT stSendEvent;
    int i;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ���콺�� ���� ��ġ�� ��ȯ
    GetCursorPosition( &iMouseX, &iMouseY );

    // �������� ũ��� ���� ����
    iWindowWidth = 500;
    iWindowHeight = 200;
    
    // ������ ���� �Լ� ȣ��, ���콺�� �ִ� ��ġ�� �������� �����ϰ� ��ȣ�� �߰��Ͽ�
    // �����츶�� �������� �̸��� �Ҵ�
    SPrintf( vcTempBuffer, "Hello World Window %d", s_iWindowCount++ );
    qwWindowID = CreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, vcTempBuffer );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    //--------------------------------------------------------------------------
    // ������ �Ŵ����� ������� �����ϴ� �̺�Ʈ�� ǥ���ϴ� ������ �׸�
    //--------------------------------------------------------------------------
    // �̺�Ʈ ������ ����� ��ġ ����
    iY = WINDOW_TITLEBAR_HEIGHT + 10;
    
    // �̺�Ʈ ������ ǥ���ϴ� ������ �׵θ��� ������ ID�� ǥ��
    DrawRect( qwWindowID, 10, iY + 8, iWindowWidth - 10, iY + 70, RGB( 0, 0, 0 ),
            FALSE );
    SPrintf( vcTempBuffer, "GUI Event Information[Window ID: 0x%Q]", qwWindowID );
    DrawText( qwWindowID, 20, iY, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), 
               vcTempBuffer, kStrLen( vcTempBuffer ) );    
    
    //--------------------------------------------------------------------------
    // ȭ�� �Ʒ��� �̺�Ʈ ���� ��ư�� �׸�
    //--------------------------------------------------------------------------
    // ��ư ������ ����
    SetRectangleData( 10, iY + 80, iWindowWidth - 10, iWindowHeight - 10, 
            &stButtonArea );
    // ����� �������� �������� �����ϰ� ���ڴ� ���������� �����Ͽ� ��ư�� �׸�
    DrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, 
            "User Message Send Button(Up)", RGB( 0, 0, 0 ) );
    // �����츦 ȭ�鿡 ǥ��
    ShowWindow( qwWindowID, TRUE );
    
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 0 );
            continue;
        }
        
        // ������ �̺�Ʈ ������ ǥ�õ� ������ �������� ĥ�Ͽ� ������ ǥ���� �����͸�
        // ��� ����
        DrawRect( qwWindowID, 11, iY + 20, iWindowWidth - 11, iY + 69, 
                   WINDOW_COLOR_BACKGROUND, TRUE );        
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 �̺�Ʈ ó��
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // ���⿡ ���콺 �̺�Ʈ ó�� �ڵ� �ֱ�
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // ���콺 �̺�Ʈ�� Ÿ���� ���
            SPrintf( vcTempBuffer, "Mouse Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            DrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // ���콺 �����͸� ���
            SPrintf( vcTempBuffer, "Data: X = %d, Y = %d, Button = %X", 
                     pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY,
                     pstMouseEvent->bButtonStatus );
            DrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );

            //------------------------------------------------------------------
            // ���콺 ���� �Ǵ� ������ �̺�Ʈ�̸� ��ư�� ������ �ٽ� �׸�
            //------------------------------------------------------------------
            // ���콺 ���� ��ư�� ������ �� ��ư ó��
            if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONDOWN )
            {
                // ��ư ������ ���콺 ���� ��ư�� ���ȴ����� �Ǵ�
                if( IsInRectangle( &stButtonArea, pstMouseEvent->stPoint.iX, 
                                    pstMouseEvent->stPoint.iY ) == TRUE )
                {
                    // ��ư�� ����� ���� ������� �����Ͽ� �������� ǥ��
                    DrawButton( qwWindowID, &stButtonArea, 
                                 RGB( 79, 204, 11 ), "User Message Send Button(Down)",
                                 RGB( 255, 255, 255 ) );
                    UpdateScreenByID( qwWindowID );
                    
                    //----------------------------------------------------------
                    // �ٸ� ������� ���� �̺�Ʈ�� ����
                    //----------------------------------------------------------
                    // ������ ��� �����츦 ã�Ƽ� �̺�Ʈ�� ����
                    stSendEvent.qwType = EVENT_USER_TESTMESSAGE;
                    stSendEvent.vqwData[ 0 ] = qwWindowID;
                    stSendEvent.vqwData[ 1 ] = 0x1234;
                    stSendEvent.vqwData[ 2 ] = 0x5678;
                    
                    // ������ �������� �� ��ŭ ������ �����ϸ鼭 �̺�Ʈ�� ����
                    for( i = 0 ; i < s_iWindowCount ; i++ )
                    {
                        // ������ �������� �����츦 �˻�
                        SPrintf( vcTempBuffer, "Hello World Window %d", i );
                        qwFindWindowID = FindWindowByTitle( vcTempBuffer );
                        // �����찡 �����ϸ� ������ �ڽ��� �ƴ� ���� �̺�Ʈ�� ����
                        if( ( qwFindWindowID != WINDOW_INVALIDID ) &&
                            ( qwFindWindowID != qwWindowID ) )
                        {
                            // ������� �̺�Ʈ ����
                            SendEventToWindow( qwFindWindowID, &stSendEvent );
                        }
                    }
                }
            }
            // ���콺 ���� ��ư�� �������� �� ��ư ó��
            else if( stReceivedEvent.qwType == EVENT_MOUSE_LBUTTONUP )
            {
                // ��ư�� ����� ������� �����Ͽ� ������ �ʾ����� ǥ��
                DrawButton( qwWindowID, &stButtonArea, 
                    WINDOW_COLOR_BACKGROUND, "User Message Send Button(Up)", 
                    RGB( 0, 0, 0 ) );
            }            
            break;

            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // ���⿡ Ű���� �̺�Ʈ ó�� �ڵ� �ֱ�
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );

            // Ű �̺�Ʈ�� Ÿ���� ���
            SPrintf( vcTempBuffer, "Key Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            DrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // Ű �����͸� ���
            SPrintf( vcTempBuffer, "Data: Key = %c, Flag = %X", 
                    pstKeyEvent->bASCIICode, pstKeyEvent->bFlags );
            DrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            // ���⿡ ������ �̺�Ʈ ó�� �ڵ� �ֱ�
            pstWindowEvent = &( stReceivedEvent.stWindowEvent );

            // ������ �̺�Ʈ�� Ÿ���� ���
            SPrintf( vcTempBuffer, "Window Event: %s", 
                      vpcEventString[ stReceivedEvent.qwType ] );
            DrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // ������ �����͸� ���
            SPrintf( vcTempBuffer, "Data: X1 = %d, Y1 = %d, X2 = %d, Y2 = %d", 
                    pstWindowEvent->stArea.iX1, pstWindowEvent->stArea.iY1, 
                    pstWindowEvent->stArea.iX2, pstWindowEvent->stArea.iY2 );
            DrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            //------------------------------------------------------------------
            // ������ �ݱ� �̺�Ʈ�̸� �����츦 �����ϰ� ������ �������� �½�ũ�� ����
            //------------------------------------------------------------------
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                DeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            // �� �� ���� �̺�Ʈ�� ���
            SPrintf( vcTempBuffer, "Unknown Event: 0x%X", stReceivedEvent.qwType );
            DrawText( qwWindowID, 20, iY + 20, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
                       vcTempBuffer, kStrLen( vcTempBuffer ) );
            
            // �����͸� ���
            SPrintf( vcTempBuffer, "Data0 = 0x%Q, Data1 = 0x%Q, Data2 = 0x%Q",
                      stReceivedEvent.vqwData[ 0 ], stReceivedEvent.vqwData[ 1 ], 
                      stReceivedEvent.vqwData[ 2 ] );
            DrawText( qwWindowID, 20, iY + 40, RGB( 0, 0, 0 ), 
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, kStrLen( vcTempBuffer ) );
            break;
        }

        // �����츦 ȭ�鿡 ������Ʈ
        ShowWindow( qwWindowID, TRUE );
    }
}

//------------------------------------------------------------------------------
//  �ý��� ����� �½�ũ
//------------------------------------------------------------------------------
/**
 *  �ý����� ���¸� �����Ͽ� ȭ�鿡 ǥ���ϴ� �½�ũ
 */
void SystemMonitorTask( void )
{
    QWORD qwWindowID;
    int i;
    int iWindowWidth;
    int iProcessorCount;
    DWORD vdwLastCPULoad[ MAXPROCESSORCOUNT ];
    int viLastTaskCount[ MAXPROCESSORCOUNT ];
    QWORD qwLastTickCount;
    EVENT stReceivedEvent;
    WINDOWEVENT* pstWindowEvent;
    BOOL bChanged;
    RECT stScreenArea;
    QWORD qwLastDynamicMemoryUsedSize;
    QWORD qwDynamicMemoryUsedSize;
    QWORD qwTemp;
    
    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }

    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ȭ�� ������ ũ�⸦ ��ȯ
    GetScreenArea( &stScreenArea );
    
    // ���� ���μ����� ������ �������� �ʺ� ���
    iProcessorCount = GetProcessorCount();
    iWindowWidth = iProcessorCount * ( SYSTEMMONITOR_PROCESSOR_WIDTH + 
            SYSTEMMONITOR_PROCESSOR_MARGIN ) + SYSTEMMONITOR_PROCESSOR_MARGIN;
    
    // �����츦 ȭ�� ����� ������ �� ȭ�鿡 ǥ������ ����. ���μ��� ������ �޸� ������ 
    // ǥ���ϴ� ������ �׸� �� ȭ�鿡 ǥ��
    qwWindowID = CreateWindow( ( stScreenArea.iX2 - iWindowWidth ) / 2, 
        ( stScreenArea.iY2 - SYSTEMMONITOR_WINDOW_HEIGHT ) / 2, 
        iWindowWidth, SYSTEMMONITOR_WINDOW_HEIGHT, WINDOW_FLAGS_DEFAULT & 
        ~WINDOW_FLAGS_SHOW, "System Monitor" );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }

    // ���μ��� ������ ǥ���ϴ� ������ 3�ȼ� �β��� ǥ���ϰ� ���ڿ��� ���
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 15, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 15, RGB( 0, 0, 0 ) );
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 16, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 16, RGB( 0, 0, 0 ) );
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 17, iWindowWidth - 5, 
            WINDOW_TITLEBAR_HEIGHT + 17, RGB( 0, 0, 0 ) );
    DrawText( qwWindowID, 9, WINDOW_TITLEBAR_HEIGHT + 8, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, "Processor Information", 21 );


    // �޸� ������ ǥ���ϴ� ������ 3�ȼ� �β��� ǥ���ϰ� ���ڿ��� ���
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50, 
            iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 50, 
            RGB( 0, 0, 0 ) );
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51, 
                iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 51, 
                RGB( 0, 0, 0 ) );
    DrawLine( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52, 
                iWindowWidth - 5, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 52, 
                RGB( 0, 0, 0 ) );
    DrawText( qwWindowID, 9, WINDOW_TITLEBAR_HEIGHT + SYSTEMMONITOR_PROCESSOR_HEIGHT + 43, 
            RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, "Memory Information", 18 );
    // �����츦 ȭ�鿡 ǥ��
    ShowWindow( qwWindowID, TRUE );
    
    // ������ ���鼭 �ý��� ������ �����Ͽ� ȭ�鿡 ǥ��
    qwLastTickCount = 0;
    
    // ���������� ������ ���μ����� ���Ͽ� �½�ũ ��, �׸��� �޸� ��뷮�� ��� 0���� ����
    MemSet( vdwLastCPULoad, 0, sizeof( vdwLastCPULoad ) );
    MemSet( viLastTaskCount, 0, sizeof( viLastTaskCount ) );
    qwLastDynamicMemoryUsedSize = 0;
    
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        //----------------------------------------------------------------------
        // �̺�Ʈ ť�� �̺�Ʈ ó��
        //----------------------------------------------------------------------
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == TRUE )
        {
            // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
            switch( stReceivedEvent.qwType )
            {
                // ������ �̺�Ʈ ó��
            case EVENT_WINDOW_CLOSE:
                //--------------------------------------------------------------
                // ������ �ݱ� �̺�Ʈ�̸� �����츦 �����ϰ� ������ �������� �½�ũ�� ����
                //--------------------------------------------------------------
                // ������ ����
                DeleteWindow( qwWindowID );
                return ;
                break;
                
                // �� �� ����
            default:
                break;
            }
        }
        
        // 0.5�ʸ��� �ѹ��� �ý��� ���¸� Ȯ��
        if( ( GetTickCount() - qwLastTickCount ) < 500 )
        {
            Sleep( 1 );
            continue;
        }

        // ���������� ������ �ð��� �ֽ����� ������Ʈ
        qwLastTickCount = GetTickCount();

        //----------------------------------------------------------------------
        // ���μ��� ���� ���
        //----------------------------------------------------------------------
        // ���μ��� ����ŭ ���Ͽ� �½�ũ ���� Ȯ���Ͽ� �޶��� ���� ������ ȭ�鿡 ������Ʈ
        for( i = 0 ; i < iProcessorCount ; i++ )
        {
            bChanged = FALSE;
            
            // ���μ��� ���� �˻�
            if( vdwLastCPULoad[ i ] != GetProcessorLoad( i ) )
            {
                vdwLastCPULoad [ i ] = GetProcessorLoad( i );
                bChanged = TRUE;
            }
            // �½�ũ �� �˻�
            else if( viLastTaskCount[ i ] != GetTaskCount( i ) )
            {
                viLastTaskCount[ i ] = GetTaskCount( i );
                bChanged = TRUE;
            }
            
            // ������ ���ؼ� ���� ������ ������ ȭ�鿡 ������Ʈ
            if( bChanged == TRUE )
            {
                // ȭ�鿡 ���� ���μ����� ���ϸ� ǥ�� 
                DrawProcessorInformation( qwWindowID, i * SYSTEMMONITOR_PROCESSOR_WIDTH + 
                    ( i + 1 ) * SYSTEMMONITOR_PROCESSOR_MARGIN, WINDOW_TITLEBAR_HEIGHT + 28,
                    i );
            }
        }
        
        //----------------------------------------------------------------------
        // ���� �޸� ���� ���
        //----------------------------------------------------------------------
        // ���� �޸��� ������ ��ȯ
        GetDynamicMemoryInformation( &qwTemp, &qwTemp, &qwTemp, 
                &qwDynamicMemoryUsedSize );
        
        // ���� ���� �Ҵ� �޸� ��뷮�� ������ �ٸ��ٸ� �޸� ������ ���
        if( qwDynamicMemoryUsedSize != qwLastDynamicMemoryUsedSize )
        {
            qwLastDynamicMemoryUsedSize = qwDynamicMemoryUsedSize;
            
            // �޸� ������ ���
            DrawMemoryInformation( qwWindowID, WINDOW_TITLEBAR_HEIGHT + 
                    SYSTEMMONITOR_PROCESSOR_HEIGHT + 60, iWindowWidth );
        }
    }
}

/**
 *  ���� ���μ����� ������ ȭ�鿡 ǥ��
 */
static void DrawProcessorInformation( QWORD qwWindowID, int iX, int iY, 
        BYTE bAPICID )
{
    char vcBuffer[ 100 ];
    RECT stArea;
    QWORD qwProcessorLoad;
    QWORD iUsageBarHeight;
    int iMiddleX;
    
    // ���μ��� ID�� ǥ��
    SPrintf( vcBuffer, "Processor ID: %d", bAPICID );
    DrawText( qwWindowID, iX + 10, iY, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, 
            vcBuffer, kStrLen( vcBuffer ) );
    
    // ���μ����� �½�ũ ������ ǥ��
    SPrintf( vcBuffer, "Task Count: %d   ", GetTaskCount( bAPICID ) );
    DrawText( qwWindowID, iX + 10, iY + 18, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
            vcBuffer, kStrLen( vcBuffer ) );

    //--------------------------------------------------------------------------
    // ���μ��� ���ϸ� ��Ÿ���� ���븦 ǥ��
    //--------------------------------------------------------------------------
    // ���μ��� ���ϸ� ǥ��
    qwProcessorLoad = GetProcessorLoad( bAPICID );
    if( qwProcessorLoad > 100 )
    {
        qwProcessorLoad = 100;
    }
    
    // ���ϸ� ǥ���ϴ� ������ ��ü�� �׵θ��� ǥ��
    DrawRect( qwWindowID, iX, iY + 36, iX + SYSTEMMONITOR_PROCESSOR_WIDTH, 
            iY + SYSTEMMONITOR_PROCESSOR_HEIGHT, RGB( 0, 0, 0 ), FALSE );

    // ���μ��� ��뷮�� ��Ÿ���� ������ ����, ( ���� ��ü�� ���� * ���μ��� ���� / 100 ) 
    iUsageBarHeight = ( SYSTEMMONITOR_PROCESSOR_HEIGHT - 40 ) * qwProcessorLoad / 100;

    // ���ϸ� ǥ���ϴ� ������ ���� ���θ� ǥ��
    // ä���� ���븦 ǥ��, �׵θ��� 2�ȼ� ���� ���� ������ �� 
    DrawRect( qwWindowID, iX + 2,
        iY + ( SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight ) - 2, 
        iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2, 
        iY + SYSTEMMONITOR_PROCESSOR_HEIGHT - 2, SYSTEMMONITOR_BAR_COLOR, TRUE );
    // �� ���븦 ǥ��, �׵θ��� 2�ȼ� ���� ���� ������ ��
    DrawRect( qwWindowID, iX + 2, iY + 38, iX + SYSTEMMONITOR_PROCESSOR_WIDTH - 2,
            iY + ( SYSTEMMONITOR_PROCESSOR_HEIGHT - iUsageBarHeight ) - 1, 
            WINDOW_COLOR_BACKGROUND, TRUE );
    
    // ���μ����� ���ϸ� ǥ��, ������ ����� ���ϰ� ǥ�õǵ��� ��
    SPrintf( vcBuffer, "Usage: %d%%", qwProcessorLoad );
    iMiddleX = ( SYSTEMMONITOR_PROCESSOR_WIDTH - 
            ( kStrLen( vcBuffer ) * FONT_ENGLISHWIDTH ) ) / 2;
    DrawText( qwWindowID, iX + iMiddleX, iY + 80, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    // ���μ��� ������ ǥ�õ� ������ �ٽ� ȭ�鿡 ������Ʈ
    SetRectangleData( iX, iY, iX + SYSTEMMONITOR_PROCESSOR_WIDTH, 
            iY + SYSTEMMONITOR_PROCESSOR_HEIGHT, &stArea );    
    UpdateScreenByWindowArea( qwWindowID, &stArea );
}

/**
 *  �޸� ������ ���
 */
static void DrawMemoryInformation( QWORD qwWindowID, int iY, int iWindowWidth )
{
    char vcBuffer[ 100 ];
    QWORD qwTotalRAMKbyteSize;
    QWORD qwDynamicMemoryStartAddress;
    QWORD qwDynamicMemoryUsedSize;
    QWORD qwUsedPercent;
    QWORD qwTemp;
    int iUsageBarWidth;
    RECT stArea;
    int iMiddleX;
    
    // Mbyte ������ �޸𸮸� Kbyte ������ ��ȯ
    qwTotalRAMKbyteSize = GetTotalRAMSize() * 1024;
    
    // �޸� ������ ǥ��
    SPrintf( vcBuffer, "Total Size: %d KB        ", qwTotalRAMKbyteSize );
    DrawText( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 3, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    // ���� �޸��� ������ ��ȯ
    GetDynamicMemoryInformation( &qwDynamicMemoryStartAddress, &qwTemp, 
            &qwTemp, &qwDynamicMemoryUsedSize );
    
    SPrintf( vcBuffer, "Used Size: %d KB        ", ( qwDynamicMemoryUsedSize + 
            qwDynamicMemoryStartAddress ) / 1024 );
    DrawText( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 10, iY + 21, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, vcBuffer, kStrLen( vcBuffer ) );
    
    //--------------------------------------------------------------------------
    // �޸� ��뷮�� ��Ÿ���� ���븦 ǥ��
    //--------------------------------------------------------------------------
    // �޸� ��뷮�� ǥ���ϴ� ������ ��ü�� �׵θ��� ǥ��
    DrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN, iY + 40,
            iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN, 
            iY + SYSTEMMONITOR_MEMORY_HEIGHT - 32, RGB( 0, 0, 0 ), FALSE );
    // �޸� ��뷮(%)�� ���
    qwUsedPercent = ( qwDynamicMemoryStartAddress + qwDynamicMemoryUsedSize ) * 
        100 / 1024 / qwTotalRAMKbyteSize;
    if( qwUsedPercent > 100 )
    {
        qwUsedPercent = 100;
    }
    
    // �޸� ��뷮�� ��Ÿ���� ������ ����, ( ���� ��ü�� ���� * �޸� ��뷮 / 100 )     
    iUsageBarWidth = ( iWindowWidth - 2 * SYSTEMMONITOR_PROCESSOR_MARGIN ) * 
        qwUsedPercent / 100;
    
    // �޸� ��뷮�� ǥ���ϴ� ������ ���� ���θ� ǥ��
    // ��ĥ�� ���븦 ǥ��, �׵θ��� 2�ȼ� ���� ���� ������ �� 
    DrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 2, iY + 42, 
            SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth, 
            iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34, SYSTEMMONITOR_BAR_COLOR, TRUE );  
    // �� ���븦 ǥ��, �׵θ��� 2�ȼ� ���� ���� ������ ��
    DrawRect( qwWindowID, SYSTEMMONITOR_PROCESSOR_MARGIN + 2 + iUsageBarWidth, 
        iY + 42, iWindowWidth - SYSTEMMONITOR_PROCESSOR_MARGIN - 2,
        iY + SYSTEMMONITOR_MEMORY_HEIGHT - 34, WINDOW_COLOR_BACKGROUND, TRUE );    
    
    // ��뷮�� ��Ÿ���� �ؽ�Ʈ ǥ��, ������ ����� ��뷮�� ǥ�õǵ��� ��
    SPrintf( vcBuffer, "Usage: %d%%", qwUsedPercent );
    iMiddleX = ( iWindowWidth - ( kStrLen( vcBuffer ) * FONT_ENGLISHWIDTH ) ) / 2;
    DrawText( qwWindowID, iMiddleX, iY + 45, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, 
            vcBuffer, kStrLen( vcBuffer ) );
    
    // �޸� ������ ǥ�õ� ������ ȭ�鿡 �ٽ� ������Ʈ
    SetRectangleData(0, iY, iWindowWidth, iY + SYSTEMMONITOR_MEMORY_HEIGHT, &stArea );
    UpdateScreenByWindowArea( qwWindowID, &stArea );
}

//------------------------------------------------------------------------------
//  GUI ������ �ܼ� �� �½�ũ
//------------------------------------------------------------------------------
// ���� ȭ�� ������ ���� �����ϴ� ����
static CHARACTER gs_vstPreviousScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

/**
 *  GUI ������ �ܼ� �� �½�ũ
 */
void GUIConsoleShellTask( void )
{
    static QWORD s_qwWindowID = WINDOW_INVALIDID;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    RECT stScreenArea;
    KEY_DATA stKEY_DATA;
    TCB* pstConsoleShellTask;
    QWORD qwConsoleShellTaskID;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    // GUI �ܼ� �� �����찡 �����ϸ� ������ �����츦 �ֻ����� ����� �½�ũ ����
    if( s_qwWindowID != WINDOW_INVALIDID )
    {
        MoveWindowToTop( s_qwWindowID );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ��ü ȭ�� ������ ũ�⸦ ��ȯ
    GetScreenArea( &stScreenArea );
    
    // �������� ũ�� ����, ȭ�� ���ۿ� ���� ������ �ִ� �ʺ�� ���̸� ����ؼ� ���
    iWindowWidth = CONSOLE_WIDTH * FONT_ENGLISHWIDTH + 4;
    iWindowHeight = CONSOLE_HEIGHT * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT + 2;
    // ������ ���� �Լ� ȣ��, ȭ�� ����� ����
    s_qwWindowID = CreateWindow( ( stScreenArea.iX2 - iWindowWidth ) / 2, 
        ( stScreenArea.iY2 - iWindowHeight ) / 2, iWindowWidth, iWindowHeight, 
        WINDOW_FLAGS_DEFAULT, "Console Shell for GUI" );
    // �����츦 �������� �������� ����
    if( s_qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }

    // �� Ŀ�ǵ带 ó���ϴ� �ܼ� �� �½�ũ�� ����
    SetConsoleShellExitFlag( FALSE );
    pstConsoleShellTask = CreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, 
        ( QWORD ) StartConsoleShell, TASK_LOADBALANCINGID );
    if( pstConsoleShellTask == NULL )
    {
        // �ܼ� �� �½�ũ ������ �����ϸ� �����츦 �����ϰ� ����
        DeleteWindow( s_qwWindowID );
        return ;
    }
    // �ܼ� �� �½�ũ�� ID�� ����
    qwConsoleShellTaskID = pstConsoleShellTask->stLink.qwID;

    // ���� ȭ�� ���۸� �ʱ�ȭ
    MemSet( gs_vstPreviousScreenBuffer, 0xFF, sizeof( gs_vstPreviousScreenBuffer ) );
    
    ShowWindow( qwConsoleShellTaskID, TRUE );

    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // ȭ�� ������ ����� ������ �����쿡 ������Ʈ
        ProcessConsoleBuffer( s_qwWindowID );
        
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( s_qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 0 );
            continue;
        }
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
            // ���⿡ Ű���� �̺�Ʈ ó�� �ڵ� �ֱ�
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            stKEY_DATA.bASCIICode = pstKeyEvent->bASCIICode;
            stKEY_DATA.bFlags = pstKeyEvent->bFlags;
            stKEY_DATA.bScanCode = pstKeyEvent->bScanCode;

            // Ű�� �׷��� ���� Ű ť�� �����Ͽ� �� �½�ũ�� �Է����� ����
            PutKeyToGUIKeyQueue( &stKEY_DATA );
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_CLOSE:
            // ������ �� �½�ũ�� ����ǵ��� ���� �÷��׸� �����ϰ� ����� ������ ���
            SetConsoleShellExitFlag( TRUE );
            while( IsTaskExist( qwConsoleShellTaskID ) == TRUE )
            {
                Sleep( 1 );
            }
            
            // �����츦 �����ϰ� ������ ID�� �ʱ�ȭ
            DeleteWindow( s_qwWindowID );
            s_qwWindowID = WINDOW_INVALIDID;            
            return ;
            
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            break;
        }
    }
}

/**
 *  ȭ�� ������ ����� ������ GUI �ܼ� �� ������ ȭ�鿡 ������Ʈ
 */
static void ProcessConsoleBuffer( QWORD qwWindowID )
{
    int i;
    int j;
    CONSOLEMANAGER* pstManager;
    CHARACTER* pstScreenBuffer;
    CHARACTER* pstPreviousScreenBuffer;
    RECT stLineArea;
    BOOL bChanged;
    static QWORD s_qwLastTickCount = 0;
    BOOL bFullRedraw;
    
    // �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ �޾� ȭ�� ������ ��巹���� �����ϰ� 
    // ���� ȭ�� ������ ��巹���� ����
    pstManager = GetConsoleManager();
    pstScreenBuffer = pstManager->pstScreenBuffer;
    pstPreviousScreenBuffer = gs_vstPreviousScreenBuffer;
    
    // ȭ���� ��ü�� ������Ʈ �� �� 5�ʰ� �������� ������ ȭ�� ��ü�� �ٽ� �׸�
    if( GetTickCount() - s_qwLastTickCount > 5000 )
    {
        s_qwLastTickCount = GetTickCount();
        bFullRedraw = TRUE;
    }
    else
    {
        bFullRedraw = FALSE;
    }

    // ȭ�� ������ ���̸�ŭ �ݺ�
    for( j = 0 ; j < CONSOLE_HEIGHT ; j++ )
    {
        // ó������ ������� ���� ������ �÷��� ����
        bChanged = FALSE;
        
        // ���� ���ο� ��ȭ�� �ִ��� ���Ͽ� ���� ���� �÷��׸� ó��
        for( i = 0 ; i < CONSOLE_WIDTH ; i++ )
        {
            // ���ڸ� ���Ͽ� �ٸ��ų� ��ü�� ���� �׷��� �ϸ� ���� ȭ�� ���ۿ�
            // ������Ʈ�ϰ� ���� ���� �÷��׸� ����
            if( ( pstScreenBuffer->bCharactor != pstPreviousScreenBuffer->bCharactor ) ||
                ( bFullRedraw == TRUE ) )
            {
                // ���ڸ� ȭ�鿡 ���
                DrawText( qwWindowID, i * FONT_ENGLISHWIDTH + 2, 
                           j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT, 
                           RGB( 255, 255, 255 ), RGB( 0, 0, 0 ),
                           &( pstScreenBuffer->bCharactor ), 1);
                
                // ���� ȭ�� ���۷� ���� �Ű� ���� ���� ���ο� ������
                // �ٸ� �����Ͱ� ������ ǥ��
                MemCpy( pstPreviousScreenBuffer, pstScreenBuffer, sizeof( CHARACTER ) );
                bChanged = TRUE;
            }
            
            pstScreenBuffer++;
            pstPreviousScreenBuffer++;
        }
        
        // ���� ���ο��� ����� �����Ͱ� �ִٸ� ���� ���θ� ȭ�鿡 ������Ʈ
        if( bChanged == TRUE )
        {
            // ���� ������ ������ ���
            SetRectangleData( 2, j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT,
                5 + FONT_ENGLISHWIDTH * CONSOLE_WIDTH, ( j + 1 ) * FONT_ENGLISHHEIGHT + 
                WINDOW_TITLEBAR_HEIGHT - 1, &stLineArea );
            // �������� �Ϻθ� ȭ�� ������Ʈ
            UpdateScreenByWindowArea( qwWindowID, &stLineArea );
        }
    }
}

//------------------------------------------------------------------------------
//  �̹��� ���(Image Viewer)
//------------------------------------------------------------------------------
/**
 *  �̹��� ��� �½�ũ
 */
void ImageViewerTask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    int iEditBoxWidth;
    RECT stEditBoxArea;
    RECT stButtonArea;
    RECT stScreenArea;
    EVENT stReceivedEvent;
    EVENT stSendEvent;
    char vcFileName[ FILESYSTEM_MAXFILENAMELENGTH + 1 ];
    int iFileNameLength;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    POINT stScreenXY;
    POINT stClientXY;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        Printf( "This task can run only GUI mode~!!!\n" );
        return ;
    }
    
    //--------------------------------------------------------------------------
    // �����츦 ����
    //--------------------------------------------------------------------------
    // ��ü ȭ�� ������ ũ�⸦ ��ȯ
    GetScreenArea( &stScreenArea );
    
    // �������� ũ�� ����, ȭ�� ���ۿ� ���� ������ �ִ� �ʺ�� ���̸� ����ؼ� ���
    iWindowWidth = FONT_ENGLISHWIDTH * FILESYSTEM_MAXFILENAMELENGTH + 165;
    iWindowHeight = 35 + WINDOW_TITLEBAR_HEIGHT + 5;
    
    // ������ ���� �Լ� ȣ��, ȭ�� ����� ����
    qwWindowID = CreateWindow( ( stScreenArea.iX2 - iWindowWidth ) / 2, 
        ( stScreenArea.iY2 - iWindowHeight ) / 2, iWindowWidth, iWindowHeight, 
        WINDOW_FLAGS_DEFAULT & ~WINDOW_FLAGS_SHOW, "Image Viewer" );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }
    
    // ���� �̸��� �Է��ϴ� ����Ʈ �ڽ� ������ ǥ��
    DrawText( qwWindowID, 5, WINDOW_TITLEBAR_HEIGHT + 6, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, "FILE NAME", 9 );
    iEditBoxWidth = FONT_ENGLISHWIDTH * FILESYSTEM_MAXFILENAMELENGTH + 4;
    SetRectangleData( 85, WINDOW_TITLEBAR_HEIGHT + 5, 85 + iEditBoxWidth, 
                       WINDOW_TITLEBAR_HEIGHT + 25, &stEditBoxArea );
    DrawRect( qwWindowID, stEditBoxArea.iX1, stEditBoxArea.iY1, 
            stEditBoxArea.iX2, stEditBoxArea.iY2, RGB( 0, 0, 0 ), FALSE );

    // ���� �̸� ���۸� ���� ����Ʈ �ڽ��� �� ���� �̸��� ǥ��
    iFileNameLength = 0;
    MemSet( vcFileName, 0, sizeof( vcFileName ) );
    DrawFileName( qwWindowID, &stEditBoxArea, vcFileName, iFileNameLength );
    
    // �̹��� ��� ��ư ������ ����
    SetRectangleData( stEditBoxArea.iX2 + 10, stEditBoxArea.iY1, 
                       stEditBoxArea.iX2 + 70, stEditBoxArea.iY2, &stButtonArea );
    DrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, "Show", 
            RGB( 0, 0, 0 ) );
    
    // �����츦 ǥ��
    ShowWindow( qwWindowID, TRUE );
    
    //--------------------------------------------------------------------------
    // GUI �½�ũ�� �̺�Ʈ ó�� ����
    //--------------------------------------------------------------------------
    while( 1 )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 0 );
            continue;
        }
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // ���콺 �̺�Ʈ ó��
        case EVENT_MOUSE_LBUTTONDOWN:
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );

            // ���콺 ���� ��ư�� �̹��� ��� ��ư ������ ���������� ����� ���� �̸��� 
            // �̿��Ͽ� �̹����� ȭ�鿡 ǥ��
            if( IsInRectangle( &stButtonArea, pstMouseEvent->stPoint.iX, 
                                pstMouseEvent->stPoint.iY ) == TRUE )
            {
                // ��ư�� ���� ������ ǥ��
                DrawButton( qwWindowID, &stButtonArea, RGB( 79, 204, 11 ), "Show", 
                            RGB( 255, 255, 255 ) );
                // ��ư�� �ִ� ������ ȭ�� ������Ʈ
                UpdateScreenByWindowArea( qwWindowID, &( stButtonArea ) );

                // �̹��� ��� �����츦 �����ϰ� �̺�Ʈ�� ó��
                if( CreateImageViewerWindowAndExecute( qwWindowID, vcFileName ) 
                        == FALSE )
                {
                    // ������ ������ �����ϸ� ��ư�� �������ٰ� �������� ȿ���� �ַ���
                    // 200ms ���
                    Sleep( 200 );
                }
                
                // ��ư�� ������ ������ ǥ��
                DrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND,
                        "Show", RGB( 0, 0, 0 ) );
                // ��ư�� �ִ� ������ ȭ�� ������Ʈ
                UpdateScreenByWindowArea( qwWindowID, &( stButtonArea ) );
            }
            break;

            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            
            // �齺���̽�(Backspace) Ű�� ���Ե� ���ڸ� ����
            if( ( pstKeyEvent->bASCIICode == KEY_BACKSPACE ) &&
                ( iFileNameLength > 0 ) )
            {
                // ���ۿ� ���Ե� ������ ���ڸ� ����
                vcFileName[ iFileNameLength ] = '\0';
                iFileNameLength--;
                
                // �Էµ� ������ ����Ʈ �ڽ��� ǥ��
                DrawFileName( qwWindowID, &stEditBoxArea, vcFileName, 
                        iFileNameLength );
            }
            // ����(Enter) Ű�� �̹��� ��� ��ư�� ���� ������ ó��
            else if( ( pstKeyEvent->bASCIICode == KEY_ENTER ) &&
                     ( iFileNameLength > 0 ) )
            {
                // ��ư�� XY ��ǥ�� ȭ�� ��ǥ�� ��ȯ�Ͽ� ���콺 �̺�Ʈ�� ��ǥ�� ���
                stClientXY.iX = stButtonArea.iX1 + 1;
                stClientXY.iY = stButtonArea.iY1 + 1;                
                ConvertPointClientToScreen( qwWindowID, &stClientXY, &stScreenXY );
                
                // �̹��� ��� ��ư�� ���콺 ���� ��ư�� ���� ��ó�� ���콺 �̺�Ʈ�� ����
                SetMouseEvent( qwWindowID, EVENT_MOUSE_LBUTTONDOWN, 
                                stScreenXY.iX + 1, stScreenXY.iY + 1, 0, &stSendEvent );
                SendEventToWindow( qwWindowID, &stSendEvent );
            }
            // ESC Ű�� ������ ���� ��ư�� ���� ������ ó��
            else if( pstKeyEvent->bASCIICode == KEY_ESC )
            {
                // ������ �ݱ� �̺�Ʈ�� ������� ����
                SetWindowEvent( qwWindowID, EVENT_WINDOW_CLOSE, &stSendEvent );
                SendEventToWindow( qwWindowID, &stSendEvent );
            }
            // �� �� Ű�� ���� �̸� ���ۿ� ������ �ִ� ��츸 ���ۿ� ����
            else if( ( pstKeyEvent->bASCIICode <= 128 ) && 
                     ( pstKeyEvent->bASCIICode != KEY_BACKSPACE ) &&
                     ( iFileNameLength < FILESYSTEM_MAXFILENAMELENGTH ) )
            {
                // �Էµ� Ű�� ���� �̸� ������ �������� ����
                vcFileName[ iFileNameLength ] = pstKeyEvent->bASCIICode;
                iFileNameLength++;
                
                // �Էµ� ������ ����Ʈ �ڽ��� ǥ��
                DrawFileName( qwWindowID, &stEditBoxArea, vcFileName, 
                        iFileNameLength );
            }            
            break;

            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_CLOSE:
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                DeleteWindow( qwWindowID );
                return ;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            break;
        }
    }
}

/**
 *  ����Ʈ �ڽ� ������ ���ڸ� ���
 */
static void DrawFileName( QWORD qwWindowID, RECT* pstArea, char *pcFileName, 
        int iNameLength )
{
    // ����Ʈ �ڽ��� ����� ��� ������� ä��
    DrawRect( qwWindowID, pstArea->iX1 + 1, pstArea->iY1 + 1, pstArea->iX2 - 1, 
               pstArea-> iY2 - 1, WINDOW_COLOR_BACKGROUND, TRUE );
    
    // ���� �̸��� ���
    DrawText( qwWindowID, pstArea->iX1 + 2, pstArea->iY1 + 2, RGB( 0, 0, 0 ), 
            WINDOW_COLOR_BACKGROUND, pcFileName, iNameLength );
    
    // ���� �̸��� ���̰� ���� �ý����� ������ �ִ� ���̰� �ƴϸ� Ŀ���� ���
    if( iNameLength < FILESYSTEM_MAXFILENAMELENGTH )
    {
        DrawText( qwWindowID, pstArea->iX1 + 2 + FONT_ENGLISHWIDTH * iNameLength,
            pstArea->iY1 + 2, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND, "_", 1 );
    }
    
    // ����Ʈ �ڽ� ������ ȭ�� ������Ʈ
    UpdateScreenByWindowArea( qwWindowID, pstArea );
}

/**
 *  JPEG ������ �о ���� ������ �����쿡 ǥ���ϰ� �̺�Ʈ�� ó��
 */
static BOOL CreateImageViewerWindowAndExecute( QWORD qwMainWindowID, 
        const char* pcFileName )
{
    struct dirent* pstEntry;
    DWORD dwFileSize;
    RECT stScreenArea;
    QWORD qwWindowID;
    WINDOW* pstWindow;
    BYTE* pbFileBuffer;
    COLOR* pstOutputBuffer;
    int iWindowWidth;
    FL_FILE* fp;
    JPEG* pstJpeg;
    EVENT stReceivedEvent;
    KEYEVENT* pstKeyEvent;
    BOOL bExit;
    
    // �ʱ�ȭ
    fp = NULL;
    pbFileBuffer = NULL;
    pstOutputBuffer = NULL;
    qwWindowID = WINDOW_INVALIDID;
    
    dwFileSize  = 0;
    
    //--------------------------------------------------------------------------
    //  ������ ���� �� �̹��� ���ڵ�
    //--------------------------------------------------------------------------
    // ���� �б�
    fp = fl_fopen( pcFileName, "rb" );
    if( fp == NULL )
    {
        Printf( "[ImageViewer] %s file open fail\n", pcFileName );
        return FALSE;
    }
    
    dwFileSize = fp->filelength;

    // �޸𸮸� ���� ũ�⸸ŭ �Ҵ��ϰ� JPEG �ڷᱸ���� �Ҵ�
    pbFileBuffer = ( BYTE* ) AllocateMemory( dwFileSize );
    pstJpeg = ( JPEG* ) AllocateMemory( sizeof( JPEG ) );
    if( ( pbFileBuffer == NULL ) || ( pstJpeg == NULL ) )
    {
        Printf( "[ImageViewer] Buffer allocation fail\n" );
        FreeMemory( pbFileBuffer );
        FreeMemory( pstJpeg );
        fl_fclose( fp );
        return FALSE;
    }
    
    // ������ ���� �� JPEG ���� �������� Ȯ��
    if( ( fl_fread( pbFileBuffer, 1, dwFileSize, fp ) != dwFileSize ) ||
        ( JPEGInit( pstJpeg, pbFileBuffer, dwFileSize ) == FALSE ) )
    {
        Printf( "[ImageViewer] Read fail or file is not JPEG format %d\n" , dwFileSize);
        FreeMemory( pbFileBuffer );
        FreeMemory( pstJpeg );
        fl_fclose( fp );
        return FALSE;
    }

    // ���ڵ� ��� ��¿� ���۸� ����
    pstOutputBuffer = AllocateMemory( pstJpeg->width * pstJpeg->height * 
                                       sizeof( COLOR ) );
    // ���ڵ带 ������ �� ���������� ó���Ǿ��ٸ� �����츦 ����
    if( ( pstOutputBuffer != NULL ) &&
        ( JPEGDecode( pstJpeg, pstOutputBuffer ) == TRUE ) )
    {
        // ��ü ȭ�� ������ ũ�⸦ ��ȯ
        GetScreenArea( &stScreenArea );
        // �����츦 ����, �̹����� ũ��� ���� ǥ������ ũ�⸦ ���
        qwWindowID = CreateWindow( ( stScreenArea.iX2 - pstJpeg->width ) / 2, 
                    ( stScreenArea.iY2 - pstJpeg->height ) / 2, pstJpeg->width, 
                    pstJpeg->height + WINDOW_TITLEBAR_HEIGHT, 
                    WINDOW_FLAGS_DEFAULT & ~WINDOW_FLAGS_SHOW | WINDOW_FLAGS_RESIZABLE, 
                    pcFileName );
    }
    
    // ������ ������ �����ϰų� ��� ���� �Ҵ� �Ǵ� ���ڵ��� �����ϸ� ����
    if( ( qwWindowID == WINDOW_INVALIDID ) || ( pstOutputBuffer == NULL ) )
    {
        Printf( "[ImageViewer] Window create fail or output buffer allocation fail\n" );
        FreeMemory( pbFileBuffer );
        FreeMemory( pstJpeg );
        FreeMemory( pstOutputBuffer );
        DeleteWindow( qwWindowID );
        return FALSE;
    }

    // �������� �ʺ� ���Ͽ� ���� ǥ���� ������ ������ ������ ȭ�� ���� ������ ���ڵ���
    // �̹����� ����
    pstWindow = GetWindowWithWindowLock( qwWindowID );
    if( pstWindow != NULL )
    {
        iWindowWidth = GetRectangleWidth( &( pstWindow->stArea ) );
        MemCpy( pstWindow->pstWindowBuffer + ( WINDOW_TITLEBAR_HEIGHT * 
                iWindowWidth ), pstOutputBuffer, pstJpeg->width * 
                pstJpeg->height * sizeof( COLOR ) );

        // ����ȭ ó��
        Unlock( &( pstWindow->stLock ) );
    }
    
    // ���� ���۸� �����ϰ� �����츦 ȭ�鿡 ǥ��
    FreeMemory( pbFileBuffer );
    ShowWindow( qwWindowID, TRUE );
    
    //--------------------------------------------------------------------------
    //  ESC Ű�� ������ �ݱ� ��ư�� ó���ϴ� ������ �̺�Ʈ ����
    //--------------------------------------------------------------------------
    // ���������� �����츦 �����Ͽ� ǥ�������� ���� �̸� �Է� ������� ����
    ShowWindow( qwMainWindowID, FALSE );
    
    bExit = FALSE;
    while( bExit == FALSE )
    {
        // �̺�Ʈ ť���� �̺�Ʈ�� ����
        if( ReceiveEventFromWindowQueue( qwWindowID, &stReceivedEvent ) == FALSE )
        {
            Sleep( 0 );
            continue;
        }
        
        // ���ŵ� �̺�Ʈ�� Ÿ�Կ� ���� ������ ó��
        switch( stReceivedEvent.qwType )
        {
            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
            pstKeyEvent = &( stReceivedEvent.stKeyEvent );
            // ESC Ű�� ������ �׸��� ǥ���ϴ� �����츦 �����ϰ� ���� �̸� �Է� �����츦
            // ǥ���� �� ����
            if( pstKeyEvent->bASCIICode == KEY_ESC )
            {
                DeleteWindow( qwWindowID );
                ShowWindow( qwMainWindowID, TRUE );
                bExit = TRUE;
            }                
            break;

            // ������ �̺�Ʈ ó��
            // ������ ũ�� ���� �̺�Ʈ�� ó��
        case EVENT_WINDOW_RESIZE:
            // ����� �����쿡 ���ڵ��� �̹����� ����
            BitBlt( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT, pstOutputBuffer, 
                     pstJpeg->width, pstJpeg->height );
            // �����츦 �ѹ� �� ǥ���Ͽ� ������ ���ο� ���۵� �̹����� ȭ�鿡 ������Ʈ
            ShowWindow( qwWindowID, TRUE );
            break;
            
            // ������ �ݱ� �̺�Ʈ�� ó��
        case EVENT_WINDOW_CLOSE:
            // �ݱ� ��ư�� ������ �̹��� ��� �����츦 �����ϰ� ���� �̸� �Է� �����츦
            // ǥ���� �� ����
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                DeleteWindow( qwWindowID );
                ShowWindow( qwMainWindowID, TRUE );
                bExit = TRUE;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            break;
        }
    }

    // JPEG �̹��� ���� ���ڵ��� ����ߴ� ���۸� ��ȯ
    FreeMemory( pstJpeg );
    FreeMemory( pstOutputBuffer );
    
    fl_fclose( fp );
    return TRUE;
}
