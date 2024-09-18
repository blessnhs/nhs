/**
 *  file    Console.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ֿܼ� ���õ� �ҽ� ����
 */

#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"
#include "ConsoleShell.h"

// �ܼ��� ������ �����ϴ� �ڷᱸ��
CONSOLEMANAGER gs_stConsoleManager = { 0, };

// �׷��� ���� �������� �� ����ϴ� ȭ�� ���� ����
static CHARACTER gs_vstScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

// �׷��� ���� �������� �� GUI �ܼ� �� ������� ���޵� Ű �̺�Ʈ�� �ܼ� �� �½�ũ�� 
// �����ϴ� ť ����
static KEY_DATA gs_vstKeyQueueBuffer[ CONSOLE_GUIKEYQUEUE_MAXCOUNT ];

/**
 *  �ܼ� �ʱ�ȭ
 */
void InitializeConsole( int iX, int iY )
{
    // �ܼ� �ڷᱸ�� �ʱ�ȭ
    MemSet( &gs_stConsoleManager, 0, sizeof( gs_stConsoleManager ) );
    // ȭ�� ���� �ʱ�ȭ
    MemSet( &gs_vstScreenBuffer, 0, sizeof( gs_vstScreenBuffer ) );
    
    if( IsGraphicMode() == FALSE )
    {
        // �׷��� ���� ������ ���� �ƴϸ� ���� �޸𸮸� ȭ�� ���۷� ����
        gs_stConsoleManager.pstScreenBuffer = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    }
    else
    {
        // �׷��� ���� ���������� �׷��� ���� ȭ�� ���۸� ����
        gs_stConsoleManager.pstScreenBuffer = gs_vstScreenBuffer;
        
        // �׷��� ��忡�� ����� Ű ť�� ���ؽ��� �ʱ�ȭ
        InitializeQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), gs_vstKeyQueueBuffer, 
                CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof( KEY_DATA ) );
        InitializeMutex( &( gs_stConsoleManager.stLock ) );
    }
    
    // Ŀ�� ��ġ ����
    SetCursor( iX, iY );
}

/**
 *  Ŀ���� ��ġ�� ����
 *      ���ڸ� ����� ��ġ�� ���� ����
 */
void SetCursor( int iX, int iY ) 
{
    int iLinearValue;
    int iOldX;
    int iOldY;
    int i;
    
    // Ŀ���� ��ġ�� ���
    iLinearValue = iY * CONSOLE_WIDTH + iX;

    // �ؽ�Ʈ ���� ���������� CRT ��Ʈ�ѷ��� Ŀ�� ��ġ�� ����
    if( IsGraphicMode() == FALSE )
    {
        // CRTC ��Ʈ�� ��巹�� ��������(��Ʈ 0x3D4)�� 0x0E�� �����Ͽ�
        // ���� Ŀ�� ��ġ �������͸� ����
        OutPortByte( VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR );
        // CRTC ��Ʈ�� ������ ��������(��Ʈ 0x3D5)�� Ŀ���� ���� ����Ʈ�� ���
        OutPortByte( VGA_PORT_DATA, iLinearValue >> 8 );
    
        // CRTC ��Ʈ�� ��巹�� ��������(��Ʈ 0x3D4)�� 0x0F�� �����Ͽ�
        // ���� Ŀ�� ��ġ �������͸� ����
        OutPortByte( VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR );
        // CRTC ��Ʈ�� ������ ��������(��Ʈ 0x3D5)�� Ŀ���� ���� ����Ʈ�� ���
        OutPortByte( VGA_PORT_DATA, iLinearValue & 0xFF );
    }
    // �׷��� ���� ���������� ȭ�� ���ۿ� ����� Ŀ���� ��ġ�� �Ű���
    else
    {
        // ������ Ŀ���� �ִ� ��ġ�� �״�� Ŀ���� ���������� Ŀ���� ����
        for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
        {
            // Ŀ���� ������ ����
            if( ( gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor == '_' ) &&
                ( gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute == 0x00 ) )
            {
                gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor = ' ';
                gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute = 
                    CONSOLE_DEFAULTTEXTCOLOR;
                break;
            }
        }
        
        // ���ο� ��ġ�� Ŀ���� ���
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bCharactor = '_';
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bAttribute = 0x00;
    }
    // ���ڸ� ����� ��ġ ������Ʈ
    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
 *  ���� Ŀ���� ��ġ�� ��ȯ
 */
void GetCursor( int *piX, int *piY )
{
    *piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    *piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

/**
 *  Printf �Լ��� ���� ����
 */
void Printf( const char* pcFormatString, ... )
{
    va_list ap;
    char vcBuffer[ 1024 * 10 ];
    int iNextPrintOffset;

    // ���� ���� ����Ʈ�� ����ؼ� vsPrintf()�� ó��
    va_start( ap, pcFormatString );
    VSPrintf( vcBuffer, pcFormatString, ap );
    va_end( ap );
    
    // ���� ���ڿ��� ȭ�鿡 ���
    iNextPrintOffset = PrintfString( vcBuffer );
    
    // Ŀ���� ��ġ�� ������Ʈ
    SetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );
}

/**
 *  \n, \t�� ���� ���ڰ� ���Ե� ���ڿ��� ����� ��, ȭ����� ���� ����� ��ġ�� 
 *  ��ȯ
 */
int PrintfString( const char* pcBuffer )
{
    CHARACTER* pstScreen;
    int i, j;
    int iLength;
    int iPrintOffset;

    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // ���ڿ��� ����� ��ġ�� ����
    iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    // ���ڿ��� ���̸�ŭ ȭ�鿡 ���
    iLength = kStrLen( pcBuffer );    
    for( i = 0 ; i < iLength ; i++ )
    {
        // ���� ó��
        if( pcBuffer[ i ] == '\n' )
        {
            // ����� ��ġ�� 80�� ��� �÷����� �ű�
            // ���� ������ ���� ���ڿ��� ����ŭ ���ؼ� ���� �������� ��ġ��Ŵ
            iPrintOffset += ( CONSOLE_WIDTH - ( iPrintOffset % CONSOLE_WIDTH ) );
        }
        // �� ó��
        else if( pcBuffer[ i ] == '\t' )
        {
            // ����� ��ġ�� 8�� ��� �÷����� �ű�
            iPrintOffset += ( 8 - ( iPrintOffset % 8 ) );
        }
        // �Ϲ� ���ڿ� ���
        else
        {
            // ���� �޸𸮿� ���ڿ� �Ӽ��� �����Ͽ� ���ڸ� ����ϰ�
            // ����� ��ġ�� �������� �̵�
            pstScreen[ iPrintOffset ].bCharactor = pcBuffer[ i ];
            pstScreen[ iPrintOffset ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            iPrintOffset++;
        }
        
        // ����� ��ġ�� ȭ���� �ִ�(80 * 25)�� ������� ��ũ�� ó��
        if( iPrintOffset >= ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) )
        {
            // ���� ������ ������ �������� �� �� ���� ����
            MemCpy( pstScreen, pstScreen + CONSOLE_WIDTH,
                     ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH * sizeof( CHARACTER ) );

            // ���� ������ ������ �������� ä��
            for( j = ( CONSOLE_HEIGHT - 1 ) * ( CONSOLE_WIDTH ) ; 
                 j < ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) ; j++ )
            {
                // ���� ���
                pstScreen[ j ].bCharactor = ' ';
                pstScreen[ j ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            }
            
            // ����� ��ġ�� ���� �Ʒ��� ������ ó������ ����
            iPrintOffset = ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH;
        }
    }
    return iPrintOffset;
}

/**
 *  ��ü ȭ���� ����
 */
void ClearScreen( void )
{
    CHARACTER* pstScreen;
    int i;
    
    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // ȭ�� ��ü�� �������� ä���, Ŀ���� ��ġ�� 0, 0���� �ű�
    for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
    {
        pstScreen[ i ].bCharactor = ' ';
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
    
    // Ŀ���� ȭ�� ������� �̵�
    SetCursor( 0, 0 );
}

/**
 *  getch() �Լ��� ����
 */
BYTE GetCh( void )
{
    KEY_DATA stData;
    
    // Ű�� ������ ������ �����
    while( 1 )
    {
        // �׷��� ��尡 �ƴ� ���� Ŀ���� Ű ť���� ���� ������
        if( IsGraphicMode() == FALSE )
        {
            // Ű ť�� �����Ͱ� ���ŵ� ������ ���
            while( GetKeyFromKeyQueue( &stData ) == FALSE )
            {
                Schedule();
            }
        }
        // �׷��� ����� ���� �׷��� ���� Ű ť���� ���� ������
        else
        {
            while( GetKeyFromGUIKeyQueue( &stData ) == FALSE )
            {
                // �׷��� ��忡�� �����ϴ� �߿� �� �½�ũ�� �����ؾߵ� ��� ������ ����
                if( gs_stConsoleManager.bExit == TRUE )
                {
                    return 0xFF;
                }
                Schedule();
            }

            if( stData.bASCIICode == KEY_BACKSPACE )
            {
	        }
        }
        
        // Ű�� ���ȴٴ� �����Ͱ� ���ŵǸ� ASCII �ڵ带 ��ȯ
        if( stData.bFlags & KEY_FLAGS_DOWN )
        {
            return stData.bASCIICode;
        }
    }
}

char* gets_s(char* str, size_t length)
{
    int32_t i = 0;
    char c;
    do
    {
        c = GetCh();

        if (c=='\b')  // Backspace
        {
            if (i>0)
            {
                //putchar(c);
                 if (i < length-2)
                    str[i-1]='\0';
                --i;
            }
        }
        else
        {
            if (c != '\n')
            {
                if (i < length-1)
                    str[i] = c;
                i++;
            }
            Printf("%c",c);
          //  putchar(c);
        }

        Schedule();
    }
    while (c != '\n'); // Linefeed

    if (i >= length)
        i = length - 1;
    str[i] = '\0';

    return str;
}

/**
 *  ���ڿ��� X, Y ��ġ�� ���
 */
void PrintStringXY( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen;
    int i;
    
    // ȭ�� ���۸� ����
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
  
    // ���� ����� ��ġ�� ���
    pstScreen += ( iY * CONSOLE_WIDTH ) + iX;
    // ���ڿ��� ���̸�ŭ ������ ���鼭 ���ڿ� �Ӽ��� ����
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
}

/**
 *  �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ
 */
CONSOLEMANAGER* GetConsoleManager( void )
{
    return &gs_stConsoleManager;
}

/**
 *  �׷��� ���� Ű ť���� Ű �����͸� ����
 */
BOOL GetKeyFromGUIKeyQueue( KEY_DATA* pstData )
{
    BOOL bResult;
    
    // ť�� �����Ͱ� ������ ����
    if( IsQueueEmpty( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // ����ȭ ó��
    Lock( &( gs_stConsoleManager.stLock ) );

    // ť���� �����͸� ����
    bResult = GetQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // ����ȭ ó��
    Unlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  �׷��� ���� Ű ť�� �����͸� ����
 */
BOOL PutKeyToGUIKeyQueue( KEY_DATA* pstData )
{
    BOOL bResult;
    
    // ť�� �����Ͱ� ���� á���� ����
    if( IsQueueFull( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // ����ȭ ó��
    Lock( &( gs_stConsoleManager.stLock ) );

    // ť�� �����͸� ����
    bResult = PutQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // ����ȭ ó��
    Unlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  �ܼ� �� �½�ũ ���� �÷��׸� ����
 */
void SetConsoleShellExitFlag( BOOL bFlag )
{
    gs_stConsoleManager.bExit = bFlag;
}



char* GUICommandShell(char *vcCommandBuffer)
{
	const int COMMAND_MAX_SIZE =  1024;
	int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;
    GetCursor( &iCursorX, &iCursorY );

    bool Exit = FALSE;
    while(Exit == FALSE )
    {

        bKey = GetCh();

        if( Exit == TRUE )
        {
            break;
        }

        if(  bKey == '`' )
        {
            break;
        }
        else if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {
                GetCursor( &iCursorX, &iCursorY );
                PrintStringXY( iCursorX - 1, iCursorY, " " );
                SetCursor( iCursorX - 1, iCursorY );
                iCommandBufferIndex--;
            }
        }
        else if( bKey == KEY_ENTER )
        {
            Printf( "\n" );

            if( iCommandBufferIndex > 0 )
            {
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                return vcCommandBuffer;
            }

            MemSet( vcCommandBuffer, '\0', COMMAND_MAX_SIZE );
            iCommandBufferIndex = 0;
        }
         else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
            ;
        }
        else if( bKey < 128 )
        {
             if( bKey == KEY_TAB )
            {
                bKey = ' ';
            }

            if( iCommandBufferIndex < COMMAND_MAX_SIZE )
            {
                vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
                Printf( "%c", bKey );
            }
        }
    }
}
