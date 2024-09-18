#include "MINTOSLibrary.h"

// �������α׷��� ������ ���� �̺�Ʈ Ÿ�� ����
#define EVENT_USER_TESTMESSAGE          0x80000001

/**
 *  �������α׷��� C ��� ��Ʈ�� ����Ʈ
 */
int Main( char* pcArgument )
{
    QWORD qwWindowID;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;


    char vcTempBuffer[1024];

    if( IsGraphicMode() == FALSE )
    {        
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        printf( "This task can run only GUI mode~!!!\n" );
        return -1;
    }

    // �������� ũ��� ���� ����
    iWindowWidth = 800;
    iWindowHeight = 600;

     qwWindowID = CreateWindow( 100, 100,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT, vcTempBuffer );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
    	printf( "CreateWindow Failed\n" );
        return -1;
    }

    int idx,iY;
    RECT stButtonArea;
    DIRECTORY *dirlist = (DIRECTORY*)dir("/");
    int count = dirinfilecount("/");
    for(idx = 0;idx<count;idx++)
    {
    	iY = WINDOW_TITLEBAR_HEIGHT + 10;

    	    // �̺�Ʈ ������ ǥ���ϴ� ������ �׵θ��� ������ ID�� ǥ��
    	DrawRect( qwWindowID, 10, iY + 8 + (idx *40), 160, iY + 40+ (idx *40), RGB( 0, 0, 0 ),
    	            TRUE );
    	sprintf( vcTempBuffer, "[%s]", dirlist[idx].filename );
    	DrawText( qwWindowID, 20, iY + 15+ (idx *40), RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
    	               vcTempBuffer, strlen( vcTempBuffer ) );
    }

    ShowWindow( qwWindowID, TRUE );

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
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:            
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
                break;
    
            // Ű �̺�Ʈ ó��
        case EVENT_KEY_DOWN:
        case EVENT_KEY_UP:
        		break;
    
            // ������ �̺�Ʈ ó��
        case EVENT_WINDOW_SELECT:
        case EVENT_WINDOW_DESELECT:
        case EVENT_WINDOW_MOVE:
        case EVENT_WINDOW_RESIZE:
        case EVENT_WINDOW_CLOSE:
            if( stReceivedEvent.qwType == EVENT_WINDOW_CLOSE )
            {
                // ������ ����
                DeleteWindow( qwWindowID );
                return 0;
            }
            break;
            
            // �� �� ����
        default:
            // ���⿡ �� �� ���� �̺�Ʈ ó�� �ڵ� �ֱ�
            // �� �� ���� �̺�Ʈ�� ���
            sprintf( vcTempBuffer, "Unknown Event: 0x%X", stReceivedEvent.qwType );
            DrawText( qwWindowID, 20, 20, RGB( 0, 0, 0 ), WINDOW_COLOR_BACKGROUND,
                       vcTempBuffer, strlen( vcTempBuffer ) );

            // �����͸� ���
            sprintf( vcTempBuffer, "Data0 = 0x%Q, Data1 = 0x%Q, Data2 = 0x%Q",
                      stReceivedEvent.vqwData[ 0 ], stReceivedEvent.vqwData[ 1 ],
                      stReceivedEvent.vqwData[ 2 ] );
            DrawText( qwWindowID, 20,40, RGB( 0, 0, 0 ),
                    WINDOW_COLOR_BACKGROUND, vcTempBuffer, strlen( vcTempBuffer ) );
            break;
        }
    
        // �����츦 ȭ�鿡 ������Ʈ
        ShowWindow( qwWindowID, TRUE );
    }

    return 0;
}

#if 0
//------------------------------------------------------------------------------
//  �⺻ GUI �������α׷�
//------------------------------------------------------------------------------
/**
 *  �⺻ GUI �������α׷��� �ڵ�
 *      GUI �������α׷��� ���� �� �����Ͽ� �⺻ �ڵ�� ���
 */
void BaseGUITask( void )
{
    QWORD qwWindowID;
    int iMouseX, iMouseY;
    int iWindowWidth, iWindowHeight;
    EVENT stReceivedEvent;
    MOUSEEVENT* pstMouseEvent;
    KEYEVENT* pstKeyEvent;
    WINDOWEVENT* pstWindowEvent;

    //--------------------------------------------------------------------------
    // �׷��� ��� �Ǵ�
    //--------------------------------------------------------------------------
    // MINT64 OS�� �׷��� ���� �����ߴ��� Ȯ��
    if( IsGraphicMode() == FALSE )
    {
        // MINT64 OS�� �׷��� ���� �������� �ʾҴٸ� ����
        printf( "This task can run only GUI mode~!!!\n" );
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

    // ������ ���� �Լ� ȣ��, ���콺�� �ִ� ��ġ�� �������� ����
    qwWindowID = CreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2,
        iWindowWidth, iWindowHeight, WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE,
         "Hello World Window" );
    // �����츦 �������� �������� ����
    if( qwWindowID == WINDOW_INVALIDID )
    {
        return ;
    }

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
        case EVENT_MOUSE_MOVE:
        case EVENT_MOUSE_LBUTTONDOWN:
        case EVENT_MOUSE_LBUTTONUP:
        case EVENT_MOUSE_RBUTTONDOWN:
        case EVENT_MOUSE_RBUTTONUP:
        case EVENT_MOUSE_MBUTTONDOWN:
        case EVENT_MOUSE_MBUTTONUP:
            // ���⿡ ���콺 �̺�Ʈ ó�� �ڵ� �ֱ�
            pstMouseEvent = &( stReceivedEvent.stMouseEvent );
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
#endif
