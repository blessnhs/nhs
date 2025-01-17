/**
 *  file    Console.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔에 관련된 소스 파일
 */

#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"
#include "ConsoleShell.h"

// 콘솔의 정보를 관리하는 자료구조
CONSOLEMANAGER gs_stConsoleManager = { 0, };

// 그래픽 모드로 시작했을 때 사용하는 화면 버퍼 영역
static CHARACTER gs_vstScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

// 그래픽 모드로 시작했을 때 GUI 콘솔 셸 윈도우로 전달된 키 이벤트를 콘솔 셸 태스크로 
// 전달하는 큐 버퍼
static KEY_DATA gs_vstKeyQueueBuffer[ CONSOLE_GUIKEYQUEUE_MAXCOUNT ];

/**
 *  콘솔 초기화
 */
void InitializeConsole( int iX, int iY )
{
    // 콘솔 자료구조 초기화
    MemSet( &gs_stConsoleManager, 0, sizeof( gs_stConsoleManager ) );
    // 화면 버퍼 초기화
    MemSet( &gs_vstScreenBuffer, 0, sizeof( gs_vstScreenBuffer ) );
    
    if( IsGraphicMode() == FALSE )
    {
        // 그래픽 모드로 시작한 것이 아니면 비디오 메모리를 화면 버퍼로 설정
        gs_stConsoleManager.pstScreenBuffer = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    }
    else
    {
        // 그래픽 모드로 시작했으면 그래픽 모드용 화면 버퍼를 설정
        gs_stConsoleManager.pstScreenBuffer = gs_vstScreenBuffer;
        
        // 그래픽 모드에서 사용할 키 큐와 뮤텍스를 초기화
        InitializeQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), gs_vstKeyQueueBuffer, 
                CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof( KEY_DATA ) );
        InitializeMutex( &( gs_stConsoleManager.stLock ) );
    }
    
    // 커서 위치 설정
    SetCursor( iX, iY );
}

/**
 *  커서의 위치를 설정
 *      문자를 출력할 위치도 같이 설정
 */
void SetCursor( int iX, int iY ) 
{
    int iLinearValue;
    int iOldX;
    int iOldY;
    int i;
    
    // 커서의 위치를 계산
    iLinearValue = iY * CONSOLE_WIDTH + iX;

    // 텍스트 모드로 시작했으면 CRT 컨트롤러로 커서 위치를 전송
    if( IsGraphicMode() == FALSE )
    {
        // CRTC 컨트롤 어드레스 레지스터(포트 0x3D4)에 0x0E를 전송하여
        // 상위 커서 위치 레지스터를 선택
        OutPortByte( VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR );
        // CRTC 컨트롤 데이터 레지스터(포트 0x3D5)에 커서의 상위 바이트를 출력
        OutPortByte( VGA_PORT_DATA, iLinearValue >> 8 );
    
        // CRTC 컨트롤 어드레스 레지스터(포트 0x3D4)에 0x0F를 전송하여
        // 하위 커서 위치 레지스터를 선택
        OutPortByte( VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR );
        // CRTC 컨트롤 데이터 레지스터(포트 0x3D5)에 커서의 하위 바이트를 출력
        OutPortByte( VGA_PORT_DATA, iLinearValue & 0xFF );
    }
    // 그래픽 모드로 시작했으면 화면 버퍼에 출력한 커서의 위치를 옮겨줌
    else
    {
        // 이전에 커서가 있던 위치가 그대로 커서로 남아있으면 커서를 지움
        for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
        {
            // 커서가 있으면 삭제
            if( ( gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor == '_' ) &&
                ( gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute == 0x00 ) )
            {
                gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor = ' ';
                gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute = 
                    CONSOLE_DEFAULTTEXTCOLOR;
                break;
            }
        }
        
        // 새로운 위치에 커서를 출력
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bCharactor = '_';
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bAttribute = 0x00;
    }
    // 문자를 출력할 위치 업데이트
    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
 *  현재 커서의 위치를 반환
 */
void GetCursor( int *piX, int *piY )
{
    *piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    *piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

/**
 *  Printf 함수의 내부 구현
 */
void Printf( const char* pcFormatString, ... )
{
    va_list ap;
    char vcBuffer[ 1024 * 10 ];
    int iNextPrintOffset;

    // 가변 인자 리스트를 사용해서 vsPrintf()로 처리
    va_start( ap, pcFormatString );
    VSPrintf( vcBuffer, pcFormatString, ap );
    va_end( ap );
    
    // 포맷 문자열을 화면에 출력
    iNextPrintOffset = PrintfString( vcBuffer );
    
    // 커서의 위치를 업데이트
    SetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );
}

/**
 *  \n, \t와 같은 문자가 포함된 문자열을 출력한 후, 화면상의 다음 출력할 위치를 
 *  반환
 */
int PrintfString( const char* pcBuffer )
{
    CHARACTER* pstScreen;
    int i, j;
    int iLength;
    int iPrintOffset;

    // 화면 버퍼를 설정
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // 문자열을 출력할 위치를 저장
    iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    // 문자열의 길이만큼 화면에 출력
    iLength = kStrLen( pcBuffer );    
    for( i = 0 ; i < iLength ; i++ )
    {
        // 개행 처리
        if( pcBuffer[ i ] == '\n' )
        {
            // 출력할 위치를 80의 배수 컬럼으로 옮김
            // 현재 라인의 남은 문자열의 수만큼 더해서 다음 라인으로 위치시킴
            iPrintOffset += ( CONSOLE_WIDTH - ( iPrintOffset % CONSOLE_WIDTH ) );
        }
        // 탭 처리
        else if( pcBuffer[ i ] == '\t' )
        {
            // 출력할 위치를 8의 배수 컬럼으로 옮김
            iPrintOffset += ( 8 - ( iPrintOffset % 8 ) );
        }
        // 일반 문자열 출력
        else
        {
            // 비디오 메모리에 문자와 속성을 설정하여 문자를 출력하고
            // 출력할 위치를 다음으로 이동
            pstScreen[ iPrintOffset ].bCharactor = pcBuffer[ i ];
            pstScreen[ iPrintOffset ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            iPrintOffset++;
        }
        
        // 출력할 위치가 화면의 최댓값(80 * 25)을 벗어났으면 스크롤 처리
        if( iPrintOffset >= ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) )
        {
            // 가장 윗줄을 제외한 나머지를 한 줄 위로 복사
            MemCpy( pstScreen, pstScreen + CONSOLE_WIDTH,
                     ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH * sizeof( CHARACTER ) );

            // 가장 마지막 라인은 공백으로 채움
            for( j = ( CONSOLE_HEIGHT - 1 ) * ( CONSOLE_WIDTH ) ; 
                 j < ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) ; j++ )
            {
                // 공백 출력
                pstScreen[ j ].bCharactor = ' ';
                pstScreen[ j ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            }
            
            // 출력할 위치를 가장 아래쪽 라인의 처음으로 설정
            iPrintOffset = ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH;
        }
    }
    return iPrintOffset;
}

/**
 *  전체 화면을 삭제
 */
void ClearScreen( void )
{
    CHARACTER* pstScreen;
    int i;
    
    // 화면 버퍼를 설정
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
    
    // 화면 전체를 공백으로 채우고, 커서의 위치를 0, 0으로 옮김
    for( i = 0 ; i < CONSOLE_WIDTH * CONSOLE_HEIGHT ; i++ )
    {
        pstScreen[ i ].bCharactor = ' ';
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
    
    // 커서를 화면 상단으로 이동
    SetCursor( 0, 0 );
}

/**
 *  getch() 함수의 구현
 */
BYTE GetCh( void )
{
    KEY_DATA stData;
    
    // 키가 눌러질 때까지 대기함
    while( 1 )
    {
        // 그래픽 모드가 아닌 경우는 커널의 키 큐에서 값을 가져옴
        if( IsGraphicMode() == FALSE )
        {
            // 키 큐에 데이터가 수신될 때까지 대기
            while( GetKeyFromKeyQueue( &stData ) == FALSE )
            {
                Schedule();
            }
        }
        // 그래픽 모드인 경우는 그래픽 모드용 키 큐에서 값을 가져옴
        else
        {
            while( GetKeyFromGUIKeyQueue( &stData ) == FALSE )
            {
                // 그래픽 모드에서 동작하는 중에 셸 태스크를 종료해야될 경우 루프를 종료
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
        
        // 키가 눌렸다는 데이터가 수신되면 ASCII 코드를 반환
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
 *  문자열을 X, Y 위치에 출력
 */
void PrintStringXY( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen;
    int i;
    
    // 화면 버퍼를 설정
    pstScreen = gs_stConsoleManager.pstScreenBuffer;
  
    // 현재 출력할 위치를 계산
    pstScreen += ( iY * CONSOLE_WIDTH ) + iX;
    // 문자열의 길이만큼 루프를 돌면서 문자와 속성을 저장
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
    }
}

/**
 *  콘솔을 관리하는 자료구조를 반환
 */
CONSOLEMANAGER* GetConsoleManager( void )
{
    return &gs_stConsoleManager;
}

/**
 *  그래픽 모드용 키 큐에서 키 데이터를 제거
 */
BOOL GetKeyFromGUIKeyQueue( KEY_DATA* pstData )
{
    BOOL bResult;
    
    // 큐에 데이터가 없으면 실패
    if( IsQueueEmpty( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // 동기화 처리
    Lock( &( gs_stConsoleManager.stLock ) );

    // 큐에서 데이터를 제거
    bResult = GetQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // 동기화 처리
    Unlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  그래픽 모드용 키 큐에 데이터를 삽입
 */
BOOL PutKeyToGUIKeyQueue( KEY_DATA* pstData )
{
    BOOL bResult;
    
    // 큐에 데이터가 가득 찼으면 실패
    if( IsQueueFull( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE )
    {
        return FALSE;
    }
    
    // 동기화 처리
    Lock( &( gs_stConsoleManager.stLock ) );

    // 큐에 데이터를 삽입
    bResult = PutQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );
    
    // 동기화 처리
    Unlock( &( gs_stConsoleManager.stLock ) );
    
    return bResult;
}

/**
 *  콘솔 셸 태스크 종료 플래그를 설정
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
