/**
 *  file    MPConfigurationTable.c
 *  date    2009/06/20
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   MP ���� ���̺�(MP Configuration Table)�� ���õ� �ҽ� ����
 */

#include "MPConfigurationTable.h"
#include "Console.h"


// MP ���� ���̺��� �����ϴ� �ڷᱸ��
static MPCONFIGRUATIONMANAGER gs_stMPConfigurationManager = { 0, };

/**
 *  BIOS�� �������� MP Floating Header�� ã�Ƽ� �� �ּҸ� ��ȯ
 */
BOOL FindMPFloatingPointerAddress( QWORD* pstAddress )
{
    char* pcMPFloatingPointer;
    QWORD qwEBDAAddress;
    QWORD qwSystemBaseMemory;

    // Ȯ�� BIOS ������ ������ �ý��� �⺻ �޸𸮸� ���
    //Printf( "Extended BIOS Data Area = [0x%X] \n", 
    //        ( DWORD ) ( *( WORD* ) 0x040E ) * 16 );
    //Printf( "System Base Address = [0x%X]\n", 
    //        ( DWORD ) ( *( WORD* ) 0x0413 ) * 1024 );
    
    // Ȯ�� BIOS ������ ������ �˻��Ͽ� MP �÷��� �����͸� ã��
    // Ȯ�� BIOS ������ ������ 0x040E���� ���׸�Ʈ�� ���� ��巹���� ã�� �� ����
    qwEBDAAddress = *( WORD* ) ( 0x040E );
    // ���׸�Ʈ�� ���� ��巹���̹Ƿ� 16�� ���Ͽ� ���� ���� ��巹���� ��ȯ
    qwEBDAAddress *= 16;
    
    for( pcMPFloatingPointer = ( char* ) qwEBDAAddress ; 
         ( QWORD ) pcMPFloatingPointer <= ( qwEBDAAddress + 1024 ) ; 
         pcMPFloatingPointer++ )
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "MP Floating Pointer Is In EBDA, [0x%X] Address\n", 
            //         ( QWORD ) pcMPFloatingPointer );
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            return 1;
        }
    }

    // �ý��� �⺻ �޸��� ���κп��� 1Kbyte �̸��� ������ �˻��Ͽ� MP �÷��� �����͸�
    // ã��
    // �ý��� �⺻ �޸𸮴� 0x0413���� Kbyte ������ ���ĵ� ���� ã�� �� ����
    qwSystemBaseMemory = *( WORD* ) 0x0413;
    // Kbyte ������ ����� ���̹Ƿ� 1024�� ���� ���� ���� ��巹���� ��ȯ
    qwSystemBaseMemory *= 1024;

    for( pcMPFloatingPointer = ( char* ) ( qwSystemBaseMemory - 1024 ) ; 
        ( QWORD ) pcMPFloatingPointer <= qwSystemBaseMemory ; 
        pcMPFloatingPointer++ )
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "MP Floating Pointer Is In System Base Memory, [0x%X] Address\n",
            //         ( QWORD ) pcMPFloatingPointer );
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            return 1;
        }
    }
    
    // BIOS�� ROM ������ �˻��Ͽ� MP �÷��� �����͸� ã��
    for( pcMPFloatingPointer = ( char* ) 0x0F0000; 
         ( QWORD) pcMPFloatingPointer < 0x0FFFFF; pcMPFloatingPointer++ )
    {
        if( MemCmp( pcMPFloatingPointer, "_MP_", 4 ) == 0 )
        {
            //Printf( "MP Floating Pointer Is In ROM, [0x%X] Address\n", 
            //         pcMPFloatingPointer );
            *pstAddress = ( QWORD ) pcMPFloatingPointer;
            return TRUE;
        }
    }

    return 0;
}

/**
 *  MP ���� ���̺��� �м��ؼ� �ʿ��� ������ �����ϴ� �Լ�
 */
BOOL AnalysisMPConfigurationTable( void )
{
    QWORD qwMPFloatingPointerAddress;
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    MPCONFIGURATIONTABLEHEADER* pstMPConfigurationHeader;
    BYTE bEntryType;
    WORD i;
    QWORD qwEntryAddress;
    PROCESSORENTRY* pstProcessorEntry;
    BUSENTRY* pstBusEntry;
    
    // �ڷᱸ�� �ʱ�ȭ
    MemSet( &gs_stMPConfigurationManager, 0, sizeof( MPCONFIGRUATIONMANAGER ) );
    gs_stMPConfigurationManager.bISABusID = 0xFF;
    
    // MP �÷��� �������� ��巹���� ����
    if( FindMPFloatingPointerAddress( &qwMPFloatingPointerAddress ) == 0 )
    {
        return 0;
    }
    
    // MP �÷��� ���̺� ����
    pstMPFloatingPointer = ( MPFLOATINGPOINTER* ) qwMPFloatingPointerAddress;
    gs_stMPConfigurationManager.pstMPFloatingPointer = pstMPFloatingPointer;
    pstMPConfigurationHeader = ( MPCONFIGURATIONTABLEHEADER* ) 
        ( ( QWORD ) pstMPFloatingPointer->dwMPConfigurationTableAddress & 0xFFFFFFFF );
    
    // PIC ��� ���� ���� ����
    if( pstMPFloatingPointer->vbMPFeatureByte[ 1 ] & 
            MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE )
    {
        gs_stMPConfigurationManager.bUsePICMode = 1;
    }    
    
    // MP ���� ���̺� ����� �⺻ MP ���� ���̺� ��Ʈ���� ���� ��巹�� ����
    gs_stMPConfigurationManager.pstMPConfigurationTableHeader = 
        pstMPConfigurationHeader;
    gs_stMPConfigurationManager.qwBaseEntryStartAddress = 
        pstMPFloatingPointer->dwMPConfigurationTableAddress + 
        sizeof( MPCONFIGURATIONTABLEHEADER );
    
    // ��� ��Ʈ���� ���鼭 ���μ����� �ھ� ���� ����ϰ� ISA ������ �˻��Ͽ� ID�� ����
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    for( i = 0 ; i < pstMPConfigurationHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType )
        {
            // ���μ��� ��Ʈ���̸� ���μ����� ���� �ϳ� ������Ŵ
        case MP_ENTRYTYPE_PROCESSOR:
            pstProcessorEntry = ( PROCESSORENTRY* ) qwEntryAddress;
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE )
            {
                gs_stMPConfigurationManager.iProcessorCount++;
            }
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
            // ���� ��Ʈ���̸� ISA �������� Ȯ���Ͽ� ����
        case MP_ENTRYTYPE_BUS:
            pstBusEntry = ( BUSENTRY* ) qwEntryAddress;
            if( MemCmp( pstBusEntry->vcBusTypeString, MP_BUS_TYPESTRING_ISA,
                    kStrLen( MP_BUS_TYPESTRING_ISA ) ) == 0 )
            {
                gs_stMPConfigurationManager.bISABusID = pstBusEntry->bBusID;
            }
            qwEntryAddress += sizeof( BUSENTRY );
            break;
            
            // ��Ÿ ��Ʈ���� �����ϰ� �̵�
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
        default:
            qwEntryAddress += 8;
            break;
        }
    }
    return TRUE;
}

/**
 *  MP ���� ���̺��� �����ϴ� �ڷᱸ���� ��ȯ
 */
MPCONFIGRUATIONMANAGER* kGetMPConfigurationManager( void )
{
    return &gs_stMPConfigurationManager;
}

/**
 *  MP ���� ���̺��� ������ ��� ȭ�鿡 ���
 */
void PrintMPConfigurationTable( void )
{
    MPCONFIGRUATIONMANAGER* pstMPConfigurationManager;
    QWORD qwMPFloatingPointerAddress;
    MPFLOATINGPOINTER* pstMPFloatingPointer;
    MPCONFIGURATIONTABLEHEADER* pstMPTableHeader;
    PROCESSORENTRY* pstProcessorEntry;
    BUSENTRY* pstBusEntry;
    IOAPICENTRY* pstIOAPICEntry;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    LOCALINTERRUPTASSIGNMENTENTRY* pstLocalAssignmentEntry;
    QWORD qwBaseEntryAddress;
    char vcStringBuffer[ 20 ];
    WORD i;
    BYTE bEntryType;
    // ȭ�鿡 ����� ���ڿ�
    char* vpcInterruptType[ 4 ] = { "INT", "NMI", "SMI", "ExtINT" };
    char* vpcInterruptFlagsPO[ 4 ] = { "Conform", "Active High", 
            "Reserved", "Active Low" };
    char* vpcInterruptFlagsEL[ 4 ] = { "Conform", "Edge-Trigger", "Reserved", 
            "Level-Trigger"};

    //==========================================================================
    // MP ���� ���̺� ó�� �Լ��� ���� ȣ���Ͽ� �ý��� ó���� �ʿ��� ������ ����
    //==========================================================================
    Printf( "================ MP Configuration Table Summary ================\n" );
    pstMPConfigurationManager = kGetMPConfigurationManager();
    if( ( pstMPConfigurationManager->qwBaseEntryStartAddress == 0 ) &&
        ( AnalysisMPConfigurationTable() == 0 ) )
    {
        Printf( "MP Configuration Table Analysis Fail\n" );
        return ;
    }
    Printf( "MP Configuration Table Analysis Success\n" );
    
    Printf( "MP Floating Pointer Address : 0x%Q\n", 
            pstMPConfigurationManager->pstMPFloatingPointer );
    Printf( "PIC Mode Support : %d\n", pstMPConfigurationManager->bUsePICMode );
    Printf( "MP Configuration Table Header Address : 0x%Q\n",
            pstMPConfigurationManager->pstMPConfigurationTableHeader );
    Printf( "Base MP Configuration Table Entry Start Address : 0x%Q\n",
            pstMPConfigurationManager->qwBaseEntryStartAddress );
    Printf( "Processor Count : %d\n", pstMPConfigurationManager->iProcessorCount );
    Printf( "ISA Bus ID : %d\n", pstMPConfigurationManager->bISABusID );

    Printf( "Press any key to continue... ('q' is exit) : " );
    if( GetCh() == 'q' )
    {
        Printf( "\n" );
        return ;
    }
    Printf( "\n" );            
    
    //==========================================================================
    // MP �÷��� ������ ������ ���
    //==========================================================================
    Printf( "=================== MP Floating Pointer ===================\n" );
    pstMPFloatingPointer = pstMPConfigurationManager->pstMPFloatingPointer;
    MemCpy( vcStringBuffer, pstMPFloatingPointer->vcSignature, 4 );
    vcStringBuffer[ 4 ] = '\0';
    Printf( "Signature : %s\n", vcStringBuffer );
    Printf( "MP Configuration Table Address : 0x%Q\n", 
            pstMPFloatingPointer->dwMPConfigurationTableAddress );
    Printf( "Length : %d * 16 Byte\n", pstMPFloatingPointer->bLength );
    Printf( "Version : %d\n", pstMPFloatingPointer->bRevision );
    Printf( "CheckSum : 0x%X\n", pstMPFloatingPointer->bCheckSum );
    Printf( "Feature Byte 1 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 0 ] );
    // MP ���� ���̺� ��� ���� ���
    if( pstMPFloatingPointer->vbMPFeatureByte[ 0 ] == 0 )
    {
        Printf( "(Use MP Configuration Table)\n" );
    }
    else
    {
        Printf( "(Use Default Configuration)\n" );
    }    
    // PIC ��� ���� ���� ���
    Printf( "Feature Byte 2 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[ 1 ] );
    if( pstMPFloatingPointer->vbMPFeatureByte[ 2 ] & 
            MP_FLOATINGPOINTER_FEATUREBYTE2_PICMODE )
    {
        Printf( "(PIC Mode Support)\n" );
    }
    else
    {
        Printf( "(Virtual Wire Mode Support)\n" );
    }
    
    //==========================================================================
    // MP ���� ���̺� ��� ������ ���
    //==========================================================================
    Printf( "\n=============== MP Configuration Table Header ===============\n" );
    pstMPTableHeader = pstMPConfigurationManager->pstMPConfigurationTableHeader;
    MemCpy( vcStringBuffer, pstMPTableHeader->vcSignature, 4 );
    vcStringBuffer[ 4 ] = '\0';
    Printf( "Signature : %s\n", vcStringBuffer );
    Printf( "Length : %d Byte\n", pstMPTableHeader->wBaseTableLength );
    Printf( "Version : %d\n", pstMPTableHeader->bRevision );
    Printf( "CheckSum : 0x%X\n", pstMPTableHeader->bCheckSum );
    MemCpy( vcStringBuffer, pstMPTableHeader->vcOEMIDString, 8 );
    vcStringBuffer[ 8 ] = '\0';
    Printf( "OEM ID String : %s\n", vcStringBuffer );
    MemCpy( vcStringBuffer, pstMPTableHeader->vcProductIDString, 12 );
    vcStringBuffer[ 12 ] = '\0';
    Printf( "Product ID String : %s\n", vcStringBuffer );
    Printf( "OEM Table Pointer : 0x%X\n", 
             pstMPTableHeader->dwOEMTablePointerAddress );
    Printf( "OEM Table Size : %d Byte\n", pstMPTableHeader->wOEMTableSize );
    Printf( "Entry Count : %d\n", pstMPTableHeader->wEntryCount );
    Printf( "Memory Mapped I/O Address Of Local APIC : 0x%X\n",
            pstMPTableHeader->dwMemoryMapIOAddressOfLocalAPIC );
    Printf( "Extended Table Length : %d Byte\n", 
            pstMPTableHeader->wExtendedTableLength );
    Printf( "Extended Table Checksum : 0x%X\n", 
            pstMPTableHeader->bExtendedTableChecksum );
    
    Printf( "Press any key to continue... ('q' is exit) : " );
    if( GetCh() == 'q' )
    {
        Printf( "\n" );
        return ;
    }
    Printf( "\n" );
    
    //==========================================================================
    // �⺻ MP ���� ���̺� ��Ʈ�� ������ ���
    //==========================================================================
    Printf( "\n============= Base MP Configuration Table Entry =============\n" );
    qwBaseEntryAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress + 
        sizeof( MPCONFIGURATIONTABLEHEADER );
    for( i = 0 ; i < pstMPTableHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwBaseEntryAddress;
        switch( bEntryType )
        {
            // ���μ��� ��Ʈ�� ���� ���
        case MP_ENTRYTYPE_PROCESSOR:
            pstProcessorEntry = ( PROCESSORENTRY* ) qwBaseEntryAddress;
            Printf( "Entry Type : Processor\n" );
            Printf( "Local APIC ID : %d\n", pstProcessorEntry->bLocalAPICID );
            Printf( "Local APIC Version : 0x%X\n", pstProcessorEntry->bLocalAPICVersion );
            Printf( "CPU Flags : 0x%X ", pstProcessorEntry->bCPUFlags );
            // Enable/Disable ���
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_ENABLE )
            {
                Printf( "(Enable, " );
            }
            else
            {
                Printf( "(Disable, " );
            }
            // BSP/AP ���
            if( pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPUFLAGS_BSP )
            {
                Printf( "BSP)\n" );
            }
            else
            {
                Printf( "AP)\n" );
            }            
            Printf( "CPU Signature : 0x%X\n", pstProcessorEntry->vbCPUSignature );
            Printf( "Feature Flags : 0x%X\n\n", pstProcessorEntry->dwFeatureFlags );

            // ���μ��� ��Ʈ���� ũ�⸸ŭ ��巹���� �������� ���� ��Ʈ���� �̵�
            qwBaseEntryAddress += sizeof( PROCESSORENTRY );
            break;

            // ���� ��Ʈ�� ���� ���
        case MP_ENTRYTYPE_BUS:
            pstBusEntry = ( BUSENTRY* ) qwBaseEntryAddress;
            Printf( "Entry Type : Bus\n" );
            Printf( "Bus ID : %d\n", pstBusEntry->bBusID );
            MemCpy( vcStringBuffer, pstBusEntry->vcBusTypeString, 6 );
            vcStringBuffer[ 6 ] = '\0';
            Printf( "Bus Type String : %s\n\n", vcStringBuffer );
            
            // ���� ��Ʈ���� ũ�⸸ŭ ��巹���� �������� ���� ��Ʈ���� �̵�
            qwBaseEntryAddress += sizeof( BUSENTRY );
            break;
            
            // I/O APIC ��Ʈ��
        case MP_ENTRYTYPE_IOAPIC:
            pstIOAPICEntry = ( IOAPICENTRY* ) qwBaseEntryAddress;
            Printf( "Entry Type : I/O APIC\n" );
            Printf( "I/O APIC ID : %d\n", pstIOAPICEntry->bIOAPICID );
            Printf( "I/O APIC Version : 0x%X\n", pstIOAPICEntry->bIOAPICVersion );
            Printf( "I/O APIC Flags : 0x%X ", pstIOAPICEntry->bIOAPICFlags );
            // Enable/Disable ���
            if( pstIOAPICEntry->bIOAPICFlags == 1 )
            {
                Printf( "(Enable)\n" );
            }
            else
            {
                Printf( "(Disable)\n" );
            }
            Printf( "Memory Mapped I/O Address : 0x%X\n\n", 
                    pstIOAPICEntry->dwMemoryMapAddress );

            // I/O APIC ��Ʈ���� ũ�⸸ŭ ��巹���� �������� ���� ��Ʈ���� �̵�
            qwBaseEntryAddress += sizeof( IOAPICENTRY );
            break;
            
            // I/O ���ͷ�Ʈ ���� ��Ʈ��
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) 
                qwBaseEntryAddress;
            Printf( "Entry Type : I/O Interrupt Assignment\n" );
            Printf( "Interrupt Type : 0x%X ", pstIOAssignmentEntry->bInterruptType );
            // ���ͷ�Ʈ Ÿ�� ���
            Printf( "(%s)\n", vpcInterruptType[ pstIOAssignmentEntry->bInterruptType ] );
            Printf( "I/O Interrupt Flags : 0x%X ", pstIOAssignmentEntry->wInterruptFlags );
            // �ؼ��� Ʈ���� ��� ���
            Printf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstIOAssignmentEntry->wInterruptFlags & 0x03 ], 
                    vpcInterruptFlagsEL[ ( pstIOAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );
            Printf( "Source BUS ID : %d\n", pstIOAssignmentEntry->bSourceBUSID );
            Printf( "Source BUS IRQ : %d\n", pstIOAssignmentEntry->bSourceBUSIRQ );
            Printf( "Destination I/O APIC ID : %d\n", 
                     pstIOAssignmentEntry->bDestinationIOAPICID );
            Printf( "Destination I/O APIC INTIN : %d\n\n", 
                     pstIOAssignmentEntry->bDestinationIOAPICINTIN );

            // I/O ���ͷ�Ʈ ���� ��Ʈ���� ũ�⸸ŭ ��巹���� �������� ���� ��Ʈ���� �̵�
            qwBaseEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
            
            // ���� ���ͷ�Ʈ ���� ��Ʈ��
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            pstLocalAssignmentEntry = ( LOCALINTERRUPTASSIGNMENTENTRY* )
                qwBaseEntryAddress;
            Printf( "Entry Type : Local Interrupt Assignment\n" );
            Printf( "Interrupt Type : 0x%X ", pstLocalAssignmentEntry->bInterruptType );
            // ���ͷ�Ʈ Ÿ�� ���
            Printf( "(%s)\n", vpcInterruptType[ pstLocalAssignmentEntry->bInterruptType ] );
            Printf( "I/O Interrupt Flags : 0x%X ", pstLocalAssignmentEntry->wInterruptFlags );
            // �ؼ��� Ʈ���� ��� ���
            Printf( "(%s, %s)\n", vpcInterruptFlagsPO[ pstLocalAssignmentEntry->wInterruptFlags & 0x03 ], 
                    vpcInterruptFlagsEL[ ( pstLocalAssignmentEntry->wInterruptFlags >> 2 ) & 0x03 ] );
            Printf( "Source BUS ID : %d\n", pstLocalAssignmentEntry->bSourceBUSID );
            Printf( "Source BUS IRQ : %d\n", pstLocalAssignmentEntry->bSourceBUSIRQ );
            Printf( "Destination Local APIC ID : %d\n", 
                     pstLocalAssignmentEntry->bDestinationLocalAPICID );
            Printf( "Destination Local APIC LINTIN : %d\n\n", 
                     pstLocalAssignmentEntry->bDestinationLocalAPICLINTIN );
            
            // ���� ���ͷ�Ʈ ���� ��Ʈ���� ũ�⸸ŭ ��巹���� �������� ���� ��Ʈ���� �̵�
            qwBaseEntryAddress += sizeof( LOCALINTERRUPTASSIGNMENTENTRY );
            break;
            
        default :
            Printf( "Unknown Entry Type. %d\n", bEntryType );
            break;
        }

        // 3���� ����ϰ� ���� Ű �Է��� ���
        if( ( i != 0 ) && ( ( ( i + 1 ) % 3 ) == 0 ) )
        {
            Printf( "Press any key to continue... ('q' is exit) : " );
            if( GetCh() == 'q' )
            {
                Printf( "\n" );
                return ;
            }
            Printf( "\n" );            
        }        
    }
}

/**
 *  ISA ������ ����� I/O APIC ��Ʈ���� �˻�
 *      AnalysisMPConfigurationTable() �Լ��� ���� ȣ���� �ڿ� ����ؾ� ��
 */
IOAPICENTRY* FindIOAPICEntryForISA( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOAPICENTRY* pstIOAPICEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BOOL bFind = 0;
    int i;
    
    // MP ���� ���̺� ����� ���� ��巹���� ��Ʈ���� ���� ��巹���� ����
    pstMPHeader = gs_stMPConfigurationManager.pstMPConfigurationTableHeader;
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    
    //==========================================================================
    // ISA ������ ���õ� I/O ���ͷ�Ʈ ���� ��Ʈ���� �˻�
    //==========================================================================
    // ��� ��Ʈ���� ���鼭 ISA ������ ���õ� I/O ���ͷ�Ʈ ���� ��Ʈ���� �˻�
    for( i = 0 ; ( i < pstMPHeader->wEntryCount ) &&
                 ( bFind == 0 ) ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType )
        {
            // ���μ��� ��Ʈ���� ����
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
            // ���� ��Ʈ��, I/O APIC ��Ʈ��, ���� ���ͷ�Ʈ ���� ��Ʈ���� ����
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
            
            // IO ���ͷ�Ʈ ���� ��Ʈ���̸�, ISA ������ ���õ� ��Ʈ������ Ȯ��
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;
            // MP Configuration Manager �ڷᱸ���� ����� ISA ���� ID�� ��
            if( pstIOAssignmentEntry->bSourceBUSID == 
                gs_stMPConfigurationManager.bISABusID )
            {
                bFind = 1;
            }                    
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        }
    }

    // ������� �Դµ� �� ã�Ҵٸ� NULL�� ��ȯ
    if( bFind == FALSE )
    {
        return 0;
    }
    
    //==========================================================================
    // ISA ������ ���õ� I/O APIC�� �˻��Ͽ� I/O APIC�� ��Ʈ���� ��ȯ
    //==========================================================================
    // �ٽ� ��Ʈ���� ���鼭 IO ���ͷ�Ʈ ���� ��Ʈ���� ����� I/O APIC�� ID�� ��ġ�ϴ�
    // ��Ʈ���� �˻�
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    for( i = 0 ; i < pstMPHeader->wEntryCount ; i++ )
    {
        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType )
        {
            // ���μ��� ��Ʈ���� ����
        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );
            break;
            
            // ���� ��Ʈ��, IO ���ͷ�Ʈ ���� ��Ʈ��, ���� ���ͷ�Ʈ ���� ��Ʈ���� ����
        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;
            break;
            
            // I/O APIC ��Ʈ���̸� ISA ������ ����� ��Ʈ������ Ȯ���Ͽ� ��ȯ
        case MP_ENTRYTYPE_IOAPIC:
            pstIOAPICEntry = ( IOAPICENTRY* ) qwEntryAddress;
            if( pstIOAPICEntry->bIOAPICID == pstIOAssignmentEntry->bDestinationIOAPICID )
            {
                return pstIOAPICEntry;
            }
            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            break;
        }
    }
    
    return 0;
}

/**
 *  ���μ��� �Ǵ� �ھ��� ������ ��ȯ
 */
int GetProcessorCount( void )
{
    // MP ���� ���̺��� ���� ���� �����Ƿ�, 0���� ������ ��� 1�� ��ȯ
    if( gs_stMPConfigurationManager.iProcessorCount == 0 )
    {
        return 1;
    }
    return gs_stMPConfigurationManager.iProcessorCount;
}
