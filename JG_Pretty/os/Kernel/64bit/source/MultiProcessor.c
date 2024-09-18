/**
 *  file    MultiProcessor.c
 *  date    2009/06/29
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ��Ƽ ���μ��� �Ǵ� ��Ƽ�ھ� ���μ����� ���õ� �ҽ� ����
 */

#include "MultiProcessor.h"
#include "MPConfigurationTable.h"
#include "AssemblyUtility.h"
#include "LocalAPIC.h"
#include "PIT.h"

// Ȱ��ȭ�� Application Processor�� ����
volatile int g_iWakeUpApplicationProcessorCount = 0;
// APIC ID ���������� ��巹��
volatile QWORD g_qwAPICIDAddress = 0;

/**
 *  ���� APIC�� Ȱ��ȭ�ϰ� AP(Application Processor)�� Ȱ��ȭ
 */
BOOL StartUpApplicationProcessor( void )
{
    // MP ���� ���̺� �м�
    if( AnalysisMPConfigurationTable() == FALSE )
    {
        return FALSE;
    }
    
    // ��� ���μ������� ���� APIC�� ����ϵ��� Ȱ��ȭ
    EnableGlobalLocalAPIC();
    
    // BSP(Bootstrap Processor)�� ���� APIC�� Ȱ��ȭ
    EnableSoftwareLocalAPIC();    
    
    // AP�� ����
    if( WakeUpApplicationProcessor() == FALSE )
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 *  AP(Application Processor)�� Ȱ��ȭ
 */
static BOOL WakeUpApplicationProcessor( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;
    BOOL bInterruptFlag;
    int i;

    // ���ͷ�Ʈ�� �Ұ��� ����
    bInterruptFlag = SetInterruptFlag( FALSE );

    // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
    pstMPManager = kGetMPConfigurationManager(); 
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;

    // APIC ID ���������� ��巹��(0xFEE00020)�� �����Ͽ�, Application Processor��
    // �ڽ��� APIC ID�� ���� �� �ְ� ��
    g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
    
    //--------------------------------------------------------------------------
    // ���� ���ͷ�Ʈ Ŀ�ǵ� ��������(Lower Interrupt Command Register, 0xFEE00300)��
    // �ʱ�ȭ(INIT) IPI�� ����(Start Up) IPI�� �����Ͽ� AP�� ����
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    // �ʱ�ȭ(INIT) IPI ����
    //--------------------------------------------------------------------------
    // ���� ���ͷ�Ʈ Ŀ�ǵ� ��������(0xFEE00300)�� ����ؼ� BSP(Bootstrap Processor)��
    // ������ ������ �ھ INIT IPI�� ����
    // AP(Application Processor)�� ��ȣ ��� Ŀ��(0x10000)���� ����
    // All Excluding Self, Edge Trigger, Assert, Physical Destination, INIT
    *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
        APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
        APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERYMODE_INIT;
    
    // PIT�� ���� �����Ͽ� 10ms ���� ���
    WaitUsingDirectPIT( MSTOCOUNT( 10 ) );
        
    // ���� ���ͷ�Ʈ Ŀ�ǵ� ��������(0xFEE00300)���� ���� ���� ��Ʈ(��Ʈ 12)�� 
    // Ȯ���Ͽ� ���� ���� �Ǻ�
    if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &
            APIC_DELIVERYSTATUS_PENDING )
    {
        // Ÿ�̸� ���ͷ�Ʈ�� 1�ʿ� 1000�� �߻��ϵ��� �缳��
        InitializePIT( MSTOCOUNT( 1 ), TRUE );
        
        // ���ͷ�Ʈ �÷��׸� ����
        SetInterruptFlag( bInterruptFlag );
        return FALSE;
    }
    
    //--------------------------------------------------------------------------
    // ����(Start Up) IPI ����, 2ȸ �ݺ� ������
    //--------------------------------------------------------------------------
    for( i = 0 ; i < 2 ; i++ )
    {
        // ���� ���ͷ�Ʈ Ŀ�ǵ� ��������(0xFEE00300)�� ����ؼ� BSP�� ������ 
        // ������ �ھ SIPI �޽����� ����
        // ��ȣ ��� Ŀ���� �����ϴ� 0x10000���� �����Ű���� 0x10(0x10000 / 4Kbyte)��
        // ���ͷ�Ʈ ���ͷ� ����
        // All Excluding Self, Edge Trigger, Assert, Physical Destination, Start Up
        *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = 
            APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE |
            APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | 
            APIC_DELIVERYMODE_STARTUP | 0x10;
        
        // PIT�� ���� �����Ͽ� 200us ���� ���
        WaitUsingDirectPIT( USTOCOUNT( 200 ) );
            
        // ���� ���ͷ�Ʈ Ŀ�ǵ� ��������(0xFEE00300)���� ���� ���� ��Ʈ(��Ʈ 12)�� 
        // Ȯ���Ͽ� ���� ���� �Ǻ�
        if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) &
                APIC_DELIVERYSTATUS_PENDING )
        {
            // Ÿ�̸� ���ͷ�Ʈ�� 1�ʿ� 1000�� �߻��ϵ��� �缳��
            InitializePIT( MSTOCOUNT( 1 ), TRUE );
            
            // ���ͷ�Ʈ �÷��׸� ����
            SetInterruptFlag( bInterruptFlag );
            return FALSE;
        }
    }
    
    // Ÿ�̸� ���ͷ�Ʈ�� 1�ʿ� 1000�� �߻��ϵ��� �缳��
    InitializePIT( MSTOCOUNT( 1 ), TRUE );
    
    // ���ͷ�Ʈ �÷��׸� ����
    SetInterruptFlag( bInterruptFlag );
    
    // Application Processor�� ��� ��� ������ ���
    while( g_iWakeUpApplicationProcessorCount < 
            ( pstMPManager->iProcessorCount - 1 ) )
    {
        Sleep( 50 );
    }    
    return TRUE;
}

/**
 *  APIC ID �������Ϳ��� APIC ID�� ��ȯ
 */
BYTE GetAPICID( void )
{
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;

    // APIC ID ��巹���� ���� �������� �ʾ�����, MP ���� ���̺��� ���� �о ����
    if( g_qwAPICIDAddress == 0 )
    {
        // MP ���� ���̺� ����� ����� ���� APIC�� �޸� �� I/O ��巹���� ���
        pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
        if( pstMPHeader == NULL )
        {
            return 0;
        }
        
        // APIC ID ���������� ��巹��((0xFEE00020)�� �����Ͽ�, �ڽ��� APIC ID��
        // ���� �� �ְ� ��
        qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
        g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;
    }
    
    BYTE APIC = *( ( DWORD* ) g_qwAPICIDAddress ) >> 24;

    // APIC ID ���������� Bit 24~31�� ���� ��ȯ    
    return APIC % MAXPROCESSORCOUNT;
}
