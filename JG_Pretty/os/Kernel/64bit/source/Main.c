/**
 *  file    Main.c
 *  date    2017/09/18
 *  author  blessnhs
 *          Copyright(c)2008 All rights reserved by blessnhs
 *  brief
 */

#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "SerialPort.h"
#include "MultiProcessor.h"
#include "VBE.h"
#include "Graphics.h"
#include "MPConfigurationTable.h"
#include "WindowManagerTask.h"
#include "SystemCall.h"
#include "PCI.h"
#include "usb/usb.h"
#include "utility.h"
#include "fat32/fat_filelib.h"
#include "NETPROTOCOL/fd.h"
#include "NETPROTOCOL/if.h"
#include "NETPROTOCOL/socket.h"
#include "MODULE/ftp.h"

// Application Processor�� ���� Main �Լ�
void MainForApplicationProcessor( void );
// ��Ƽ �ھ� ���μ��� �Ǵ� ��Ƽ ���μ��� ���� ��ȯ�ϴ� �Լ�
BOOL kChangeToMultiCoreMode( void );
// �׷��� ��带 �׽�Ʈ�ϴ� �Լ�
void StartGraphicModeTest( void );

void InitNetwork( void );

/**
 *  Bootstrap Processor�� C ��� Ŀ�� ��Ʈ�� ����Ʈ
 *      �Ʒ� �Լ��� C ��� Ŀ���� ���� �κ���
 */
void Main( void )
{
    int iCursorX, iCursorY;
    BYTE bButton;
    int iX;
    int iY;
    
    // ��Ʈ �δ��� �ִ� BSP �÷��׸� �о Application Processor�̸� 
    // �ش� �ھ�� �ʱ�ȭ �Լ��� �̵�
    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 )
    {
        MainForApplicationProcessor();
    }
    
    // Bootstrap Processor�� ������ �Ϸ������Ƿ�, 0x7C09�� �ִ� Bootstrap Processor��
    // ��Ÿ���� �÷��׸� 0���� �����Ͽ� Application Processor������ �ڵ� ���� ��θ� ����
    *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) = 0;

    // �ܼ��� ���� �ʱ�ȭ�� ��, ���� �۾��� ����
    InitializeConsole( 0, 10 );    
    Printf( "Switch To IA-32e Mode Success~!!\n" );
    Printf( "IA-32e C Language Kernel Start..............[Pass]\n" );
    Printf( "Initialize Console..........................[Pass]\n" );
    
    // ���� ��Ȳ�� ȭ�鿡 ���
    GetCursor( &iCursorX, &iCursorY );
    Printf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    InitializeGDTTableAndTSS();
    LoadGDTR( GDTR_STARTADDRESS );
    SetCursor( 45, iCursorY++ );
    Printf( "Pass\n" );
    
    Printf( "TSS Segment Load............................[    ]" );
    LoadTR( GDT_TSSSEGMENT );
    SetCursor( 45, iCursorY++ );
    Printf( "Pass\n" );
    
    Printf( "IDT Initialize..............................[    ]" );
    InitializeIDTTables();    
    LoadIDTR( IDTR_STARTADDRESS );
    SetCursor( 45, iCursorY++ );
    Printf( "Pass\n" );
    
    Printf( "Total RAM Size Check........................[    ]" );
    CheckTotalRAMSize();
    SetCursor( 45, iCursorY++ );
    Printf( "Pass], Size = %d MB\n", GetTotalRAMSize() );
    
    Printf( "TCB Pool And Scheduler Initialize...........[Pass]\n" );
    iCursorY++;
    InitializeScheduler();
    
    // ���� �޸� �ʱ�ȭ
    Printf( "Dynamic Memory Initialize...................[Pass]\n" );
    iCursorY++;
    InitializeDynamicMemory();
    
    // 1ms�� �ѹ��� ���ͷ�Ʈ�� �߻��ϵ��� ����
    InitializePIT( MSTOCOUNT( 1 ), 1 );
    
    Printf( "Keyboard Activate And Queue Initialize......[    ]" );
    // Ű���带 Ȱ��ȭ
    if( InitializeKeyboard() == TRUE )
    {
        SetCursor( 45, iCursorY++ );
        Printf( "Pass\n" );
        ChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        SetCursor( 45, iCursorY++ );
        Printf( "Fail\n" );
    }

    Printf( "Mouse Activate And Queue Initialize.........[    ]" );
    // ���콺�� Ȱ��ȭ
    if( InitializeMouse() == TRUE )
    {
        EnableMouseInterrupt();
        SetCursor( 45, iCursorY++ );
        Printf( "Pass\n" );
    }
    else
    {
        SetCursor( 45, iCursorY++ );
        Printf( "Fail\n" );
    }

    Printf( "PIC Controller And Interrupt Initialize.....[    ]" );
    // PIC ��Ʈ�ѷ� �ʱ�ȭ �� ��� ���ͷ�Ʈ Ȱ��ȭ
    InitializePIC();
    MaskPICInterrupt( 0 );
    EnableInterrupt();
    SetCursor( 45, iCursorY++ );
    Printf( "Pass\n" );
    
    // �ø��� ��Ʈ�� �ʱ�ȭ
    Printf( "Serial Port Initialize......................[Pass]\n" );
    iCursorY++;
    InitializeSerialPort();
    
    // ��Ƽ�ھ� ���μ��� ���� ��ȯ
    // Application Processor Ȱ��ȭ, I/O ��� Ȱ��ȭ, ���ͷ�Ʈ�� �½�ũ ���� �л�
    // ��� Ȱ��ȭ
    Printf( "Change To MultiCore Processor Mode..........[    ]" );
    if( kChangeToMultiCoreMode() == TRUE )
    {
        SetCursor( 45, iCursorY++ );
        Printf( "Pass\n" );
    }
    else
    {
        SetCursor( 45, iCursorY++ );
        Printf( "Fail\n" );
    }
    // �ý��� �ݿ� ���õ� MSR�� �ʱ�ȭ
    Printf( "System Call MSR Initialize..................[Pass]\n" );
    iCursorY++;
    InitializeSystemCall();

    InitNetwork();

    pci_scan();
	pci_installDevices();


	netif_t *netif = netif_findbyname ("eth0");
	if(netif != 0)
		dhcp_config_if(netif);
	else
		Printf( " Fail dhcp_config_if\n");

	//init end
	iflist_display();


//	connectServer("ftp.twaren.net",21);

    CreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD , 0, 0,
                     ( QWORD ) usb_pollInterruptTransfers, GetAPICID() );

    //CreateTask( TASK_FLAGS_HIGHEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM , 0, 0,
    //                 ( QWORD ) deviceManager_checkDrives, GetAPICID() );

    CreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0,
                     ( QWORD ) IdleTask, GetAPICID() );

    // �׷��� ��尡 �ƴϸ� �ܼ� �� ����
    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 )
    {
        StartConsoleShell();
    }
    // �׷��� ����̸� ������ �Ŵ��� ���� ����
    else
    {
        StartWindowManager();
    }
}

/**
 *  Application Processor�� C ��� Ŀ�� ��Ʈ�� ����Ʈ
 *      ��κ��� �ڷᱸ���� Bootstrap Processor�� ������ �������Ƿ� �ھ �����ϴ�
 *      �۾��� ��
 */
void MainForApplicationProcessor( void )
{
    QWORD qwTickCount;

    // GDT ���̺��� ����
    LoadGDTR( GDTR_STARTADDRESS );

    // TSS ��ũ���͸� ����. TSS ���׸�Ʈ�� ��ũ���͸� Application Processor�� 
    // ����ŭ ���������Ƿ�, APIC ID�� �̿��Ͽ� TSS ��ũ���͸� �Ҵ�
    LoadTR( GDT_TSSSEGMENT + ( GetAPICID() * sizeof( GDTENTRY16 ) ) );

    // IDT ���̺��� ����
    LoadIDTR( IDTR_STARTADDRESS );
    
    // �����ٷ� �ʱ�ȭ
    InitializeScheduler();
    
    // ���� �ھ��� ���� APIC�� Ȱ��ȭ
    EnableSoftwareLocalAPIC();

    // ��� ���ͷ�Ʈ�� ������ �� �ֵ��� �½�ũ �켱 ���� �������͸� 0���� ����
    SetTaskPriority( 0 );

    // ���� APIC�� ���� ���� ���̺��� �ʱ�ȭ
    InitializeLocalVectorTable();

    // ���ͷ�Ʈ�� Ȱ��ȭ
    EnableInterrupt();    

    // �ý��� �ݿ� ���õ� MSR�� �ʱ�ȭ
    InitializeSystemCall();

    // ��Ī I/O ��� �׽�Ʈ�� ���� Application Processor�� ������ �� �ѹ��� ���
    //Printf( "Application Processor[APIC ID: %d] Is Activated\n",
    //        GetAPICID() );

    // ���� �½�ũ ����
    IdleTask();
}

/**
 *  ��Ƽ�ھ� ���μ��� �Ǵ� ��Ƽ ���μ��� ���� ��ȯ�ϴ� �Լ�
 */
BOOL kChangeToMultiCoreMode( void )
{
    MPCONFIGRUATIONMANAGER* pstMPManager;
    BOOL bInterruptFlag;
    int i;

    // Application Processor Ȱ��ȭ
    if( StartUpApplicationProcessor() == FALSE )
    {
        return FALSE;
    }

    //--------------------------------------------------------------------------
    // ��Ī I/O ���� ��ȯ
    //--------------------------------------------------------------------------
    // MP ���� �Ŵ����� ã�Ƽ� PIC ����ΰ� Ȯ��
    pstMPManager = kGetMPConfigurationManager();
    if( pstMPManager->bUsePICMode == TRUE )
    {
        // PIC ����̸� I/O ��Ʈ ��巹�� 0x22�� 0x70�� ���� �����ϰ� 
        // I/O ��Ʈ ��巹�� 0x23�� 0x01�� �����ϴ� ������� IMCR �������Ϳ� �����Ͽ�
        // PIC ��� ��Ȱ��ȭ
        OutPortByte( 0x22, 0x70 );
        OutPortByte( 0x23, 0x01 );
    }

    // PIC ��Ʈ�ѷ��� ���ͷ�Ʈ�� ��� ����ũ�Ͽ� ���ͷ�Ʈ�� �߻��� �� ������ ��
    MaskPICInterrupt( 0xFFFF );

    // ���μ��� ��ü�� ���� APIC�� Ȱ��ȭ
    EnableGlobalLocalAPIC();
    
    // ���� �ھ��� ���� APIC�� Ȱ��ȭ
    EnableSoftwareLocalAPIC();

    // ���ͷ�Ʈ�� �Ұ��� ����
    bInterruptFlag = SetInterruptFlag( FALSE );
    
    // ��� ���ͷ�Ʈ�� ������ �� �ֵ��� �½�ũ �켱 ���� �������͸� 0���� ����
    SetTaskPriority( 0 );

    // ���� APIC�� ���� ���� ���̺��� �ʱ�ȭ
    InitializeLocalVectorTable();

    // ��Ī I/O ���� ����Ǿ����� ����
    SetSymmetricIOMode( TRUE );
    
    // I/O APIC �ʱ�ȭ
    InitializeIORedirectionTable();
        
    // ���� ���ͷ�Ʈ �÷��׸� ����
    SetInterruptFlag( bInterruptFlag );

    // ���ͷ�Ʈ ���� �л� ��� Ȱ��ȭ
    SetInterruptLoadBalancing( TRUE );

    // �½�ũ ���� �л� ��� Ȱ��ȭ
    for( i = 0 ; i < MAXPROCESSORCOUNT ; i++ )
    {
        SetTaskLoadBalancing( i, TRUE );
    }
    
    return TRUE;
}

void InitNetwork( void )
{
	//init network layer
	init_fd();
	init_netif();	/* Network interface */
	init_socket();	/* Network socket API */
	init_packet();	/* Network handler - traffic parser */

}
