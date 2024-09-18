/**
 *  file    Descriptor.c
 *  date    2009/01/16
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   GDT �� IDT�� ���õ� ���� ��ũ���Ϳ� ���� ����
 */

#include "Descriptor.h"
#include "Utility.h"
#include "ISR.h"
#include "MultiProcessor.h"

//==============================================================================
//  GDT �� TSS
//==============================================================================

/**
 *  GDT ���̺��� �ʱ�ȭ
 */
void InitializeGDTTableAndTSS( void )
{
    GDTR* pstGDTR;
    GDTENTRY8* pstEntry;
    TSSSEGMENT* pstTSS;
    int i;
    
    // GDTR ����
    pstGDTR = ( GDTR* ) GDTR_STARTADDRESS;
    pstEntry = ( GDTENTRY8* ) ( GDTR_STARTADDRESS + sizeof( GDTR ) );
    pstGDTR->wLimit = GDT_TABLESIZE - 1;
    pstGDTR->qwBaseAddress = ( QWORD ) pstEntry;
    
    // TSS ���׸�Ʈ ���� ����, GDT ���̺��� ���ʿ� ��ġ
    pstTSS = ( TSSSEGMENT* ) ( ( QWORD ) pstEntry + GDT_TABLESIZE );

   // NULL, Ŀ�� �ڵ�/������, ���� �ڵ�/������, TSS�� ���� �� 5 + 16���� ���׸�Ʈ�� ����
    SetGDTEntry8( &( pstEntry[ 0 ] ), 0, 0, 0, 0, 0 );
    // Ŀ�� ���� �ڵ�/������ ��ũ���� ����
    SetGDTEntry8( &( pstEntry[ 1 ] ), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, 
            GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE );
    SetGDTEntry8( &( pstEntry[ 2 ] ), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA,
            GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA );
    // ���� ���� �ڵ�/������ ��ũ���� ����
    SetGDTEntry8( &( pstEntry[ 3 ] ), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA,
            GDT_FLAGS_LOWER_USERDATA, GDT_TYPE_DATA );
    SetGDTEntry8( &( pstEntry[ 4 ] ), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE, 
            GDT_FLAGS_LOWER_USERCODE, GDT_TYPE_CODE );
    
    // 16�� �ھ� ������ ���� 16���� TSS ��ũ���͸� ����
    for( i = 0 ; i < MAXPROCESSORCOUNT ; i++ )
    {
        // TSS�� 16����Ʈ�̹Ƿ�, SetGDTEntry16() �Լ� ���
        // pstEntry�� 8����Ʈ�̹Ƿ� 2���� ���ļ� �ϳ��� ���
        SetGDTEntry16( ( GDTENTRY16* ) &( pstEntry[ GDT_MAXENTRY8COUNT + 
                ( i * 2 ) ] ), ( QWORD ) pstTSS + ( i * sizeof( TSSSEGMENT ) ), 
                sizeof( TSSSEGMENT ) - 1, GDT_FLAGS_UPPER_TSS, 
                GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS ); 
    }
    
    // TSS �ʱ�ȭ, GDT ���� ������ �����
    InitializeTSSSegment( pstTSS );    
}

/**
 *  8����Ʈ ũ���� GDT ��Ʈ���� ���� ����
 *      �ڵ�� ������ ���׸�Ʈ ��ũ���͸� �����ϴµ� ���
 */
void SetGDTEntry8( GDTENTRY8* pstEntry, DWORD dwBaseAddress, DWORD dwLimit,
        BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType )
{
    pstEntry->wLowerLimit = dwLimit & 0xFFFF;
    pstEntry->wLowerBaseAddress = dwBaseAddress & 0xFFFF;
    pstEntry->bUpperBaseAddress1 = ( dwBaseAddress >> 16 ) & 0xFF;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ( ( dwLimit >> 16 ) & 0xFF ) | 
        bUpperFlags;
    pstEntry->bUpperBaseAddress2 = ( dwBaseAddress >> 24 ) & 0xFF;
}

/**
 *  16����Ʈ ũ���� GDT ��Ʈ���� ���� ����
 *      TSS ���׸�Ʈ ��ũ���͸� �����ϴµ� ���
 */
void SetGDTEntry16( GDTENTRY16* pstEntry, QWORD qwBaseAddress, DWORD dwLimit,
        BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType )
{
    pstEntry->wLowerLimit = dwLimit & 0xFFFF;
    pstEntry->wLowerBaseAddress = qwBaseAddress & 0xFFFF;
    pstEntry->bMiddleBaseAddress1 = ( qwBaseAddress >> 16 ) & 0xFF;
    pstEntry->bTypeAndLowerFlag = bLowerFlags | bType;
    pstEntry->bUpperLimitAndUpperFlag = ( ( dwLimit >> 16 ) & 0xFF ) | 
        bUpperFlags;
    pstEntry->bMiddleBaseAddress2 = ( qwBaseAddress >> 24 ) & 0xFF;
    pstEntry->dwUpperBaseAddress = qwBaseAddress >> 32;
    pstEntry->dwReserved = 0;
}

/**
 *  TSS ���׸�Ʈ�� ������ �ʱ�ȭ
 */
void InitializeTSSSegment( TSSSEGMENT* pstTSS )
{
    int i;
    
    // �ִ� ���μ��� �Ǵ� �ھ��� ����ŭ ������ ���鼭 ����
    for( i = 0 ; i < MAXPROCESSORCOUNT ; i++ )
    {
        // 0���� �ʱ�ȭ
        MemSet( pstTSS, 0, sizeof( TSSSEGMENT ) );

        // IST�� �ڿ������� �߶� �Ҵ���. (����, IST�� 16����Ʈ ������ �����ؾ� ��)
        pstTSS->qwIST[ 0 ] = IST_STARTADDRESS + IST_SIZE - 
            ( IST_SIZE / MAXPROCESSORCOUNT * i );
        
        // IO Map�� ���� �ּҸ� TSS ��ũ������ Limit �ʵ庸�� ũ�� ���������ν� 
        // IO Map�� ������� �ʵ��� ��
        pstTSS->wIOMapBaseAddress = 0xFFFF;

        // ���� ��Ʈ���� �̵�
        pstTSS++;
    }
}


//==============================================================================
//  IDT
//==============================================================================
/**
 *  IDT ���̺��� �ʱ�ȭ
 */
void InitializeIDTTables( void )
{
    IDTR* pstIDTR;
    IDTENTRY* pstEntry;
    int i;
        
    // IDTR�� ���� ��巹��
    pstIDTR = ( IDTR* ) IDTR_STARTADDRESS;
    // IDT ���̺��� ���� ����
    pstEntry = ( IDTENTRY* ) ( IDTR_STARTADDRESS + sizeof( IDTR ) );
    pstIDTR->qwBaseAddress = ( QWORD ) pstEntry;
    pstIDTR->wLimit = IDT_TABLESIZE - 1;
    
    //==========================================================================
    // ���� ISR ���
    //==========================================================================
    SetIDTEntry( &( pstEntry[ 0 ] ), ISRDivideError, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 1 ] ), ISRDebug, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 2 ] ), ISRNMI, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 3 ] ), ISRBreakPoint, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 4 ] ), ISROverflow, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 5 ] ), ISRBoundRangeExceeded, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 6 ] ), ISRInvalidOpcode, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 7 ] ), ISRDeviceNotAvailable, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 8 ] ), ISRDoubleFault, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 9 ] ), ISRCoprocessorSegmentOverrun, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 10 ] ), ISRInvalidTSS, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 11 ] ), ISRSegmentNotPresent, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 12 ] ), ISRStackSegmentFault, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 13 ] ), ISRGeneralProtection, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 14 ] ), ISRPageFault, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 15 ] ), ISR15, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 16 ] ), ISRFPUError, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 17 ] ), ISRAlignmentCheck, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 18 ] ), ISRMachineCheck, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 19 ] ), ISRSIMDError, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 20 ] ), ISRETCException, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    
    for( i = 21 ; i < 32 ; i++ )
    {
        SetIDTEntry( &( pstEntry[ i ] ), ISRETCException, 0x08, IDT_FLAGS_IST1,
            IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    }
    //==========================================================================
    // ���ͷ�Ʈ ISR ���
    //==========================================================================
    SetIDTEntry( &( pstEntry[ 32 ] ), ISRTimer, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 33 ] ), ISRKeyboard, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 34 ] ), ISRSlavePIC, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 35 ] ), ISRSerial2, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 36 ] ), ISRSerial1, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 37 ] ), ISRParallel2, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 38 ] ), ISRFloppy, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 39 ] ), ISRParallel1, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 40 ] ), ISRRTC, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 41 ] ), ISRReserved, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 42 ] ), ISRNotUsed1, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 43 ] ), ISRNotUsed2, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 44 ] ), ISRMouse, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 45 ] ), ISRCoprocessor, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 46 ] ), ISRHDD1, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    SetIDTEntry( &( pstEntry[ 47 ] ), ISRHDD2, 0x08, IDT_FLAGS_IST1,
        IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );

    for( i = 48 ; i < IDT_MAXENTRYCOUNT ; i++ )
    {
        SetIDTEntry( &( pstEntry[ i ] ), ISRETCInterrupt, 0x08, IDT_FLAGS_IST1,
            IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT );
    }
}

/**
 *  IDT ����Ʈ ��ũ���Ϳ� ���� ����
 */
void SetIDTEntry( IDTENTRY* pstEntry, void* pvHandler, WORD wSelector, 
        BYTE bIST, BYTE bFlags, BYTE bType )
{
    pstEntry->wLowerBaseAddress = ( QWORD ) pvHandler & 0xFFFF;
    pstEntry->wSegmentSelector = wSelector;
    pstEntry->bIST = bIST & 0x3;
    pstEntry->bTypeAndFlags = bType | bFlags;
    pstEntry->wMiddleBaseAddress = ( ( QWORD ) pvHandler >> 16 ) & 0xFFFF;
    pstEntry->dwUpperBaseAddress = ( QWORD ) pvHandler >> 32;
    pstEntry->dwReserved = 0;
}
