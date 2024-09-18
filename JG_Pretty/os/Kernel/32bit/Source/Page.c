
#include "Page.h"
#include "../../64bit/Source/Task.h"

// 02.Kernel64/Source/DynamicMemory.h�� �ִ� ���� �޸� ������ ���� ��ġ�� �Űܿ�
// ���� �޸��� ���� ��ġ�� �����Ǹ� ���⵵ ���� �����ؾ� ��
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + 0x1fffff ) & \
                                          0xffe00000 )

/**
 *  IA-32e ��� Ŀ���� ���� ������ ���̺� ����
 */
void InitializePageTables( void )
{
    PML4TENTRY* pstPML4TEntry;
    PDPTENTRY* pstPDPTEntry;
    PDENTRY* pstPDEntry;
    DWORD dwMappingAddress;
    DWORD dwPageEntryFlags;
    int i;

    // PML4 ���̺� ����
     pstPML4TEntry = ( PML4TENTRY* ) 0x100000;
    // ������ ��Ʈ���� ���� ������ �����Ͽ� ���� �������� ���� �����ϵ��� ����
    SetPageEntryData( &( pstPML4TEntry[ 0 ] ), 0x00, 0x101000,
            PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US , 0 );
    for( i = 1 ; i < PAGE_MAXENTRYCOUNT ; i++ )
    {
    	SetPageEntryData( &( pstPML4TEntry[ i ] ), 0, 0, 0, 0 );
    }
    
    // ������ ���͸� ������ ���̺� ����
    // �ϳ��� PDPT�� 512GByte���� ���� �����ϹǷ� �ϳ��� �����
    // 64���� ��Ʈ���� �����Ͽ� 64GByte���� ������
    pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
    for( i = 0 ; i < 64 ; i++ )
    {
        // ������ ��Ʈ���� ���� ������ �����Ͽ� ���� �������� ���� �����ϵ��� ����
    	SetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0x102000 + ( i * PAGE_TABLESIZE ),
                PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US , 0 );
    }
    for( i = 64 ; i < PAGE_MAXENTRYCOUNT ; i++ )
    {
    	SetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0, 0, 0 );
    }
    
    // ������ ���͸� ���̺� ����
    // �ϳ��� ������ ���͸��� 1GByte���� ���� ���� 
    // �����ְ� 64���� ������ ���͸��� �����Ͽ� �� 64GB���� ����
    pstPDEntry = ( PDENTRY* ) 0x102000;
    dwMappingAddress = 0;
    for( i = 0 ; i < PAGE_MAXENTRYCOUNT * 64 ; i++ )
    {
        // ������ ���� ������ ��Ʈ���� �Ӽ��� ����
        // ���� �޸� ������ �����ϴ� ���������� Ŀ�� �����̹Ƿ� �������� �Ӽ���
        // ������(Supervisor)�� �����ϰ� �� �̻��� ����(User)�� ����
        if( i < ( ( DWORD ) DYNAMICMEMORY_START_ADDRESS / PAGE_DEFAULTSIZE ) )
        {
            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS  | PAGE_FLAGS_PCD | PAGE_FLAGS_US;
        }
        else
        {
            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS | PAGE_FLAGS_US | PAGE_FLAGS_PCD;
        }
        
        // 32��Ʈ�δ� ���� ��巹���� ǥ���� �� �����Ƿ�, Mbyte ������ ����� ����
        // ���� ����� �ٽ� 4Kbyte�� ������ 32��Ʈ �̻��� ��巹���� �����
        SetPageEntryData( &( pstPDEntry[ i ] ),
                ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, 
                dwPageEntryFlags, 0 );

        dwMappingAddress += PAGE_DEFAULTSIZE;

    }   
}

/**
 *  ������ ��Ʈ���� ���� �ּҿ� �Ӽ� �÷��׸� ����
 */
void SetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress,
        DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags )
{
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | 
        dwUpperFlags;
}
