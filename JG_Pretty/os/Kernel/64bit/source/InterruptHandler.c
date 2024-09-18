
#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"
#include "Mouse.h"

// ���ͷ�Ʈ �ڵ鷯 �ڷᱸ��
static INTERRUPTMANAGER gs_stInterruptManager;

/**
 *  ���ͷ�Ʈ �ڷᱸ�� �ʱ�ȭ
 */
void InitializeHandler( void )
{
    MemSet( &gs_stInterruptManager, 0, sizeof( gs_stInterruptManager ) );
}

/**
 *  ���ͷ�Ʈ ó�� ��带 ����
 */
void SetSymmetricIOMode( BOOL bSymmetricIOMode )
{
    gs_stInterruptManager.bSymmetricIOMode = bSymmetricIOMode;
}

/**
 *  ���ͷ�Ʈ ���� �л� ����� ������� ���θ� ����
 */
void SetInterruptLoadBalancing( BOOL bUseLoadBalancing )
{
    gs_stInterruptManager.bUseLoadBalancing = bUseLoadBalancing;
}

/**
 *  �ھ� �� ���ͷ�Ʈ ó�� Ƚ���� ����
 */
void IncreaseInterruptCount( int iIRQ )
{
    // �ھ��� ���ͷ�Ʈ ī��Ʈ�� ����
    gs_stInterruptManager.vvqwCoreInterruptCount[ GetAPICID() ][ iIRQ ]++;
}

/**
 *  ���� ���ͷ�Ʈ ��忡 ���߾� EOI�� ����
 */
void SendEOI( int iIRQ )
{
    // ��Ī I/O ��尡 �ƴϸ� PIC ����̹Ƿ�, PIC ��Ʈ�ѷ��� EOI�� �����ؾ� ��
    if( gs_stInterruptManager.bSymmetricIOMode == FALSE )
    {
        SendEOIToPIC( iIRQ );
    }
    // ��Ī I/O ����̸� ���� APIC�� EOI�� �����ؾ� ��
    else
    {
        SendEOIToLocalAPIC();
    }
}

/**
 *  ���ͷ�Ʈ �ڵ鷯 �ڷᱸ���� ��ȯ
 */
INTERRUPTMANAGER* kGetInterruptManager( void )
{
    return &gs_stInterruptManager;
}

/**
 *  ���ͷ�Ʈ ���� �л�(Interrupt Load Balancing) ó��
 */
void ProcessLoadBalancing( int iIRQ )
{
    QWORD qwMinCount = 0xFFFFFFFFFFFFFFFF;
    int iMinCountCoreIndex;
    int iCoreCount;
    int i;
    BOOL bResetCount = FALSE;
    BYTE bAPICID;
    
    bAPICID = GetAPICID();

    // ���� �л� ����� ���� �ְų�, ���� �л��� ó���� ������ �ƴϸ� �� �ʿ䰡 ����
    if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] == 0 ) ||
        ( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] % 
            INTERRUPT_LOADBALANCINGDIVIDOR ) != 0 ) ||
        ( gs_stInterruptManager.bUseLoadBalancing == FALSE ) )
    {
        return ;
    }
    
    // �ھ��� ������ ���ؼ� ������ �����ϸ� ���ͷ�Ʈ ó�� Ƚ���� ���� ���� �ھ 
    // ����
    iMinCountCoreIndex = 0;
    iCoreCount = GetProcessorCount();
    for( i = 0 ; i < iCoreCount ; i++ )
    {
        if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] <
                qwMinCount ) )
        {
            qwMinCount = gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ];
            iMinCountCoreIndex = i;
        }
        // ��ü ī��Ʈ�� ���� �ִ� ���� �����ߴٸ� ���߿� ī��Ʈ�� ��� 0���� ����
        else if( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] >=
            0xFFFFFFFFFFFFFFFE )
        {
            bResetCount = TRUE;
        }
    }
    
    // I/O �����̷��� ���̺��� �����Ͽ� ���� ���ͷ�Ʈ�� ó���� Ƚ���� ���� ���� APIC�� ����
    RoutingIRQToAPICID( iIRQ, iMinCountCoreIndex );
    
    // ó���� �ھ��� ī��Ʈ�� �ִ񰪿� �����ߴٸ� ��ü ī��Ʈ�� �ٽ� 0���� �����ϵ���
    // ����
    if( bResetCount == TRUE )
    {
        for( i = 0 ; i < iCoreCount ; i++ )
        {
            gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] = 0;
        }
    }
}

/**
 *  �������� ����ϴ� ���� �ڵ鷯
 */
void CommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 100 ];
    BYTE bAPICID;
    TCB* pstTask;
    
    // ���� ���ܰ� �߻��� �ھ ��ȯ
    bAPICID = GetAPICID();
    // ���� �ھ�� ���� ���� �½�ũ�� ��ȯ
    pstTask = GetRunningTask( bAPICID );
    

    // �޽��� ���
    PrintStringXY( 0, 0, "====================================================" );
    PrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    // ���� ���Ϳ� �ھ� ID, �����ڵ带 ���
    SPrintf( vcBuffer,   "     Vector:%d     Core ID:0x%X     ErrorCode:0x%X  ", 
            iVectorNumber, bAPICID, qwErrorCode );            
    PrintStringXY( 0, 2, vcBuffer );    
    // �½�ũ ID�� ���
    SPrintf( vcBuffer,   "                Task ID:0x%Q", pstTask->stLink.qwID );
    PrintStringXY( 0, 3, vcBuffer );
    PrintStringXY( 0, 4, "====================================================" );

    // ���� ���� �½�ũ�� ���� ���� ������ �������� �ʰ� �½�ũ�� �����Ű�� �ٸ�
    // �½�ũ�� ��ȯ
    if( pstTask->qwFlags & TASK_FLAGS_USERLEVEL )
    {
        // �½�ũ ����
        EndTask( pstTask->stLink.qwID );
        
        // ���� ���� ���� 
        // EndTask() �Լ����� �ٸ� �½�ũ�� ��ȯ�ϹǷ� ������ ����� ������� ����
        while( 1 )
        {
            ;
        }
    }
    // Ŀ�� ������ ���� ���� ���� ����
    else
    {
        // ���� ���� ����
        while( 1 )
        {
            ;
        }
    }
}


/**
 *  �������� ����ϴ� ���ͷ�Ʈ �ڵ鷯 
 */
void CommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    int iIRQ;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    PrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    irq_handler(iIRQ);

    // EOI ����
    SendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    IncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    ProcessLoadBalancing( iIRQ );
}

/**
 *  Ű���� ���ͷ�Ʈ�� �ڵ鷯
 */
void KeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;
    
    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    PrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
    // Ű ť �Ǵ� ���콺 ť�� ����
    if( IsOutputBufferFull() == TRUE )
    {
        // ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
        if( IsMouseDataInOutputBuffer() == FALSE )
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = GetKeyboardScanCode();
            // Ű ť�� ����
            ConvertScanCodeAndPutQueue( bTemp );
        }
        // ���콺 �������̸� ���콺 ť�� ����
        else
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = GetKeyboardScanCode();
            // ���콺 ť�� ����
            AccumulateMouseDataAndPutQueue( bTemp );
        } 
    }
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    SendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    IncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    ProcessLoadBalancing( iIRQ );
}

/**
 *  Ÿ�̸� ���ͷ�Ʈ�� �ڵ鷯
 */
void TimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    int iIRQ;
    BYTE bCurrentAPICID;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    PrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    SendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    IncreaseInterruptCount( iIRQ );

    // IRQ 0 ���ͷ�Ʈ ó���� Bootstrap Processor�� ó��
    bCurrentAPICID = GetAPICID();
    if( bCurrentAPICID == 0 )
    {
        // Ÿ�̸� �߻� Ƚ���� ����
        g_qwTickCount++;
    }

    // �½�ũ�� ����� ���μ����� �ð��� ����
    DecreaseProcessorTime( bCurrentAPICID );
    // ���μ����� ����� �� �ִ� �ð��� �� ��ٸ� �½�ũ ��ȯ ����
    if( IsProcessorTimeExpired( bCurrentAPICID ) == TRUE )
    {
        ScheduleInInterrupt();
    }
}

/**
 *  Device Not Available ������ �ڵ鷯
 */
void DeviceNotAvailableHandler( int iVectorNumber )
{
    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;
    BYTE bCurrentAPICID;

    //=========================================================================
    // FPU ���ܰ� �߻������� �˸����� �޽����� ����ϴ� �κ�
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    // ���� ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;  PrintStringXY( 0, 0, vcBuffer );
    //=========================================================================
    
    // ���� �ھ��� ���� APIC ID�� Ȯ��
    bCurrentAPICID = GetAPICID();
    
    // CR0 ��Ʈ�� ���������� TS ��Ʈ�� 0���� ����
    ClearTS();

    // ������ FPU�� ����� �½�ũ�� �ִ��� Ȯ���Ͽ�, �ִٸ� FPU�� ���¸� �½�ũ�� ����
    qwLastFPUTaskID = GetLastFPUUsedTaskID( bCurrentAPICID );
    pstCurrentTask = GetRunningTask( bCurrentAPICID );
    
    // ������ FPU�� ����� ���� �ڽ��̸� �ƹ��͵� �� ��
    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    {
        return ;
    }
    // FPU�� ����� �½�ũ�� ������ FPU ���¸� ����
    else if( qwLastFPUTaskID != TASK_INVALIDID )
    {
        pstFPUTask = GetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        {
            SaveFPUContext( pstFPUTask->vqwFPUContext );
        }
    }
    
    // ���� �½�ũ�� FPU�� ����� ���� �ִ� �� Ȯ���Ͽ� FPU�� ����� ���� ���ٸ� 
    // �ʱ�ȭ�ϰ�, ��������� �ִٸ� ����� FPU ���ؽ�Ʈ�� ����
    if( pstCurrentTask->bFPUUsed == FALSE )
    {
        InitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        LoadFPUContext( pstCurrentTask->vqwFPUContext );
    }
    
    // FPU�� ����� �½�ũ ID�� ���� �½�ũ�� ����
    SetLastFPUUsedTaskID( bCurrentAPICID, pstCurrentTask->stLink.qwID );
}

/**
 *  �ϵ� ��ũ���� �߻��ϴ� ���ͷ�Ʈ�� �ڵ鷯
 */
void HDDHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    // ���� ���� �ִ� �޽����� ��ġ�� �ʵ��� (10, 0)�� ���
    PrintStringXY( 10, 0, vcBuffer );
    //=========================================================================

    // ù ��° PATA ��Ʈ�� ���ͷ�Ʈ �߻� ���θ� TRUE�� ����
    SetHDDInterruptFlag( TRUE, TRUE );
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    SendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    IncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    ProcessLoadBalancing( iIRQ );
}

/**
 *  ���콺 ���ͷ�Ʈ�� �ڵ鷯
 */
void MouseHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iMouseInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;
    
    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iMouseInterruptCount;
    g_iMouseInterruptCount = ( g_iMouseInterruptCount + 1 ) % 10;
    PrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
    // Ű ť �Ǵ� ���콺 ť�� ����
    if( IsOutputBufferFull() == TRUE )
    {
        // ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
        if( IsMouseDataInOutputBuffer() == FALSE )
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = GetKeyboardScanCode();
            // Ű ť�� ����
            ConvertScanCodeAndPutQueue( bTemp );
        }
        // ���콺 �������̸� ���콺 ť�� ����
        else
        {
            // ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
            // �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
            bTemp = GetKeyboardScanCode();
            // ���콺 ť�� ����
            AccumulateMouseDataAndPutQueue( bTemp );
        } 
    }
    
    // ���ͷ�Ʈ ���Ϳ��� IRQ ��ȣ ����
    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    // EOI ����
    SendEOI( iIRQ );
    
    // ���ͷ�Ʈ �߻� Ƚ���� ������Ʈ
    IncreaseInterruptCount( iIRQ );
    
    // ���� �л�(Load Balancing) ó��
    ProcessLoadBalancing( iIRQ );
}

