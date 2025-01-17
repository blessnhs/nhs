/**
 *  file    ConsoleShell.h
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   콘솔 셸에 관련된 헤더 파일
 */

#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"
#include "fat32/fat_filelib.h"
////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "JG64>"

// 문자열 포인터를 파라미터로 받는 함수 포인터 타입 정의
typedef void ( * CommandFunction ) ( const char* pcParameter );

// 패키지의 시그너처
#define PACKAGESIGNATURE    "MINT64OSPACKAGE "

// 파일 이름의 최대 길이, 커널의 FILESYSTEM_MAXFILENAMELENGTH와 같음
#define MAXFILENAMELENGTH   24



////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 1바이트로 정렬
#pragma pack( push, 1 )

// 셸의 커맨드를 저장하는 자료구조
typedef struct kShellCommandEntryStruct
{
    // 커맨드 문자열
    char* pcCommand;
    // 커맨드의 도움말
    char* pcHelp;
    // 커맨드를 수행하는 함수의 포인터
    CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

// 파라미터를 처리하기위해 정보를 저장하는 자료구조
typedef struct kParameterListStruct
{
    // 파라미터 버퍼의 어드레스
    const char* pcBuffer;
    // 파라미터의 길이
    int iLength;
    // 현재 처리할 파라미터가 시작하는 위치
    int iCurrentPosition;
} PARAMETERLIST;

// 패키지 헤더 내부의 각 파일 정보를 구성하는 자료구조
typedef struct PackageItemStruct
{
    // 파일 이름
    char vcFileName[ MAXFILENAMELENGTH ];

    // 파일의 크기
    DWORD dwFileLength;
} PACKAGEITEM;

// 패키지 헤더 자료구조
typedef struct PackageHeaderStruct
{
    // MINT64 OS의 패키지 파일을 나타내는 시그너처
    char vcSignature[ 16 ];

    // 패키지 헤더의 전체 크기
    DWORD dwHeaderSize;

    // 패키지 아이템의 시작 위치
    PACKAGEITEM vstItem[ 0 ];
} PACKAGEHEADER;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
// 실제 셸 코드
void StartConsoleShell( void );
void ExecuteCommand( const char* pcCommandBuffer );
void InitializeParameter( PARAMETERLIST* pstList, const char* pcParameter );
int GetNextParameter( PARAMETERLIST* pstList, char* pcParameter );

// 커맨드를 처리하는 함수
static void Help( const char* pcParameterBuffer );
static void Cls( const char* pcParameterBuffer );
static void ShowTotalRAMSize( const char* pcParameterBuffer );
static void Shutdown( const char* pcParamegerBuffer );
static void MeasureProcessorSpeed( const char* pcParameterBuffer );
static void ShowDateAndTime( const char* pcParameterBuffer );
static void ChangeTaskPriority( const char* pcParameterBuffer );
static void ShowTaskList( const char* pcParameterBuffer );
static void KillTask( const char* pcParameterBuffer );
static void CPULoad( const char* pcParameterBuffer );
static void ShowMatrix( const char* pcParameterBuffer );
static void ShowDyanmicMemoryInformation( const char* pcParameterBuffer );
static void ShowHDDInformation( const char* pcParameterBuffer );
static void ReadSector( const char* pcParameterBuffer );
static void WriteSector( const char* pcParameterBuffer );
static void MountHDD( const char* pcParameterBuffer );
static void FormatHDD( const char* pcParameterBuffer );
static void ShowFileSystemInformation( const char* pcParameterBuffer );
static void CreateFile( const char* pcParameterBuffer );
static void DeleteFile( const char* pcParameterBuffer );
static void ShowRootDirectory( const char* pcParameterBuffer );
static void WriteDataToFile( const char* pcParameterBuffer );
static void ReadDataFromFile( const char* pcParameterBuffer );
static void FlushCache( const char* pcParameterBuffer );
static void DownloadFile( const char* pcParameterBuffer );
static void ShowMPConfigurationTable( const char* pcParameterBuffer );
static void ShowIRQINTINMappingTable( const char* pcParameterBuffer );
static void ShowInterruptProcessingCount( const char* pcParameterBuffer );
static void ChangeTaskAffinity( const char* pcParameterBuffer );
static void ShowVBEModeInfo( const char* pcParameterBuffer );
static void TestSystemCall( const char* pcParameterBuffer );
static void ExecuteApplicationProgram( const char* pcParameterBuffer );
static void InstallPackage( const char* pcParameterBuffer );
static void UsbReadSector( const char* pcParameterBuffer );
static void UsbWriteSector( const char* pcParameterBuffer );
static void Ftp( const char* pcParameterBuffer );
static void CreateDirectory( const char* pcParameterBuffer );
static void Ping(const char* pcParameterBuffer );
static void SVR(const char* pcParameterBuffer );
static void Cat(const char* pcParameterBuffer );

#endif /*__CONSOLESHELL_H__*/
