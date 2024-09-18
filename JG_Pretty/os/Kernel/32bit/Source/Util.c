/*
 * Util.c
 *
 *  Created on: 2017. 9. 16.
 *      Author: user
 */
#include "Util.h"

void WriteLine( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;

    pstScreen += ( iY * 80 ) + iX;

    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {

        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}

/**
 *  IA-32e ���� Ŀ�� ������ 0���� �ʱ�ȭ
 *      1Mbyte ~ 6Mbyte���� ������ �ʱ�ȭ
 */
BOOL Initialize64KernelMemory( void )
{
    DWORD* pdwCurrentAddress;

    // �ʱ�ȭ�� ������ ��巹���� 0x100000(1MB)�� ����
    pdwCurrentAddress = ( DWORD* ) 0x100000;

    // ������ ��巹���� 0x600000(6MB)���� ������ ���鼭 4����Ʈ�� 0���� ä��
    while( ( DWORD ) pdwCurrentAddress < 0x600000 )
    {
        *pdwCurrentAddress = 0x00;

        // 0���� ������ �� �ٽ� �о��� �� 0�� ������ ������ �ش� ��巹����
        // ����ϴµ� ������ ���� ���̹Ƿ� ���̻� �������� �ʰ� ����
        if( *pdwCurrentAddress != 0 )
        {
            return FALSE;
        }

        // ���� ��巹���� �̵�
        pdwCurrentAddress++;
    }

    return TRUE;
}

/**
 *  JG OS�� �����ϱ⿡ ����� �޸𸮸� ������ �ִ��� üũ
 *      64Mbyte �̻��� �޸𸮸� ������ �ִ��� �˻�
 */
BOOL CheckSystemMemory( void )
{
   DWORD* pdwCurrentAddress;

    // 0x100000(1MB)���� �˻� ����
   pdwCurrentAddress = ( DWORD* ) 0x100000;

    // 0x4000000(64MB)���� ������ ���鼭 Ȯ��
   while( ( DWORD ) pdwCurrentAddress < 0x4000000 )
   {
       *pdwCurrentAddress = 0x12345678;

        // 0x12345678�� ������ �� �ٽ� �о��� �� 0x12345678�� ������ ������
        // �ش� ��巹���� ����ϴµ� ������ ���� ���̹Ƿ� ���̻� �������� �ʰ� ����
       if( *pdwCurrentAddress != 0x12345678 )
       {
           return FALSE;
       }

        // 1MB�� �̵��ϸ鼭 Ȯ��
       pdwCurrentAddress += ( 0x100000 / 4 );
   }
   return TRUE;
}

/**
 *  IA-32e ��� Ŀ���� 0x200000(2Mbyte) ��巹���� ����
 */
void CopyKernel64ImageTo2Mbyte( void )
{
    WORD wKernel32SectorCount, wTotalKernelSectorCount;
    DWORD* pdwSourceAddress,* pdwDestinationAddress;
    int i;

    // 0x7C05�� �� Ŀ�� ���� ��, 0x7C07�� ��ȣ ��� Ŀ�� ���� ���� ��� ����
    wTotalKernelSectorCount = *( ( WORD* ) 0x7C05 );
    wKernel32SectorCount = *( ( WORD* ) 0x7C07 );

    pdwSourceAddress = ( DWORD* ) ( 0x10000 + ( wKernel32SectorCount * 512 ) );
    pdwDestinationAddress = ( DWORD* ) 0x200000;
    // IA-32e ��� Ŀ�� ���� ũ�⸸ŭ ����
    for( i = 0 ; i < 512 * ( wTotalKernelSectorCount - wKernel32SectorCount ) / 4;
        i++ )
    {
        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;
    }
}




