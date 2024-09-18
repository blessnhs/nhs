/**
 *  file    RAMDisk.c
 *  date    2009/05/22
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �� ��ũ�� ���õ� �Լ��� ������ �ҽ� ����
 */

#include "RAMDisk.h"
#include "Utility.h"
#include "DynamicMemory.h"

// �� ��ũ�� �����ϴ� �ڷᱸ��
static RDDMANAGER gs_stRDDManager;

/**
 *  �� ��ũ ����̽� ����̹� �ʱ�ȭ �Լ�
 */
BOOL InitializeRDD( DWORD dwTotalSectorCount )
{
    // �ڷᱸ�� �ʱ�ȭ
    MemSet( &gs_stRDDManager, 0, sizeof( gs_stRDDManager ) );
    
    // �� ��ũ�� ����� �޸𸮸� �Ҵ�
    gs_stRDDManager.pbBuffer = ( BYTE* ) AllocateMemory( dwTotalSectorCount * 512 );
    if( gs_stRDDManager.pbBuffer == NULL )
    {
        return FALSE;
    }
    
    // �� ���� ���� ����ȭ ��ü�� ����
    gs_stRDDManager.dwTotalSectorCount = dwTotalSectorCount;
    InitializeMutex( &( gs_stRDDManager.stMutex ) );
    return TRUE;
}

/**
 *  �� ��ũ�� ������ ��ȯ
 */
BOOL ReadRDDInformation( BOOL bPrimary, BOOL bMaster, 
        HDDINFORMATION* pstHDDInformation )
{
    // �ڷᱸ�� �ʱ�ȭ
    MemSet( pstHDDInformation, 0, sizeof( HDDINFORMATION ) );
    
    // �� ���� ���� �ø��� ��ȣ, �׸��� �� ��ȣ�� ����
    pstHDDInformation->dwTotalSectors = gs_stRDDManager.dwTotalSectorCount;
    MemCpy( pstHDDInformation->vwSerialNumber, "0000-0000", 9 );
    MemCpy( pstHDDInformation->vwModelNumber, "MINT RAM Disk v1.0", 18 ); 

    return TRUE;
}

/**
 *  �� ��ũ���� ���� ���͸� �о ��ȯ
 */
int ReadRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    int iRealReadCount;
    
    // LBA ��巹������ ������ ���� �� �ִ� ���� ���� �о�� �� ���� ���� ���ؼ�
    // ������ ���� �� �ִ� ���� ���
    iRealReadCount = MIN( gs_stRDDManager.dwTotalSectorCount - 
                      (dwLBA + iSectorCount), iSectorCount );

    // �� ��ũ �޸𸮿��� �����͸� ������ ���� ���� ����ŭ �����ؼ� ��ȯ
    MemCpy( pcBuffer, gs_stRDDManager.pbBuffer + ( dwLBA * 512 ), 
             iRealReadCount * 512 );
    
    return iRealReadCount;
}

/**
 *  �� ��ũ�� ���� ���͸� ��
 */
int WriteRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer )
{
    int iRealWriteCount;
    
    // LBA ��巹������ ������ �� �� �ִ� ���� ���� ��� �� ���� ���� ���ؼ�
    // ������ �� �� �ִ� ���� ���
    iRealWriteCount = MIN( gs_stRDDManager.dwTotalSectorCount - 
                      (dwLBA + iSectorCount), iSectorCount );

    // �����͸� ������ �� ���� ����ŭ �� ��ũ �޸𸮿� ����
    MemCpy( gs_stRDDManager.pbBuffer + ( dwLBA * 512 ), pcBuffer, 
             iRealWriteCount * 512 );
    
    return iRealWriteCount;    
}

