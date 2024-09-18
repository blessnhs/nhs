/**
 *  file    ConsoleShell.h
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ܼ� �п� ���õ� ��� ����
 */

#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"
#include "fat32/fat_filelib.h"
////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "JG64>"

// ���ڿ� �����͸� �Ķ���ͷ� �޴� �Լ� ������ Ÿ�� ����
typedef void ( * CommandFunction ) ( const char* pcParameter );

// ��Ű���� �ñ׳�ó
#define PACKAGESIGNATURE    "MINT64OSPACKAGE "

// ���� �̸��� �ִ� ����, Ŀ���� FILESYSTEM_MAXFILENAMELENGTH�� ����
#define MAXFILENAMELENGTH   24



////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ���� Ŀ�ǵ带 �����ϴ� �ڷᱸ��
typedef struct kShellCommandEntryStruct
{
    // Ŀ�ǵ� ���ڿ�
    char* pcCommand;
    // Ŀ�ǵ��� ����
    char* pcHelp;
    // Ŀ�ǵ带 �����ϴ� �Լ��� ������
    CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

// �Ķ���͸� ó���ϱ����� ������ �����ϴ� �ڷᱸ��
typedef struct kParameterListStruct
{
    // �Ķ���� ������ ��巹��
    const char* pcBuffer;
    // �Ķ������ ����
    int iLength;
    // ���� ó���� �Ķ���Ͱ� �����ϴ� ��ġ
    int iCurrentPosition;
} PARAMETERLIST;

// ��Ű�� ��� ������ �� ���� ������ �����ϴ� �ڷᱸ��
typedef struct PackageItemStruct
{
    // ���� �̸�
    char vcFileName[ MAXFILENAMELENGTH ];

    // ������ ũ��
    DWORD dwFileLength;
} PACKAGEITEM;

// ��Ű�� ��� �ڷᱸ��
typedef struct PackageHeaderStruct
{
    // MINT64 OS�� ��Ű�� ������ ��Ÿ���� �ñ׳�ó
    char vcSignature[ 16 ];

    // ��Ű�� ����� ��ü ũ��
    DWORD dwHeaderSize;

    // ��Ű�� �������� ���� ��ġ
    PACKAGEITEM vstItem[ 0 ];
} PACKAGEHEADER;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
// ���� �� �ڵ�
void StartConsoleShell( void );
void ExecuteCommand( const char* pcCommandBuffer );
void InitializeParameter( PARAMETERLIST* pstList, const char* pcParameter );
int GetNextParameter( PARAMETERLIST* pstList, char* pcParameter );

// Ŀ�ǵ带 ó���ϴ� �Լ�
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
