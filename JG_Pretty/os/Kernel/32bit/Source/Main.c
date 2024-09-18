#include  "Types.h"
#include "Page.h"
#include "Change64Mode.h"
#include "Util.h"

// Bootstrap Processor ���ΰ� ����� ��巹��, ��Ʈ �δ� ������ ���ʿ� ��ġ
#define BOOTSTRAPPROCESSOR_FLAGADDRESS  0x7C09

void Main( void )
{
    DWORD i;
    DWORD dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[ 13 ] = { 0, };
    
    // Application Processor�̸� �Ʒ��� �ڵ带 �����ϰ� �ٷ� 64��Ʈ ���� ��ȯ
    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 )
    {
    	Change64BitKernel();
        while( 1 ) ;
    }
    
    if( CheckSystemMemory() == FALSE )
    {
    	WriteLine( 45, 4, "Fail" );
    	WriteLine( 0, 5, "Not Enough Memory~!! JG OS Requires Over "
                "64Mbyte Memory~!!" );
        while( 1 ) ;
    }
    else
    {
    	WriteLine( 45, 4, "Pass" );
    }
    
    if( Initialize64KernelMemory() == FALSE )
    {
    	WriteLine( 45, 5, "Fail" );
    	WriteLine( 0, 6, "Kernel Area Initialization Fail~!!" );
        while( 1 ) ;
    }

    InitializePageTables();
    
    // ���μ��� ������ ���� �б�
    ReadCPUID( 0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX );
    *( DWORD* ) vcVendorString = dwEBX;
    *( ( DWORD* ) vcVendorString + 1 ) = dwEDX;
    *( ( DWORD* ) vcVendorString + 2 ) = dwECX;
    
    // 64��Ʈ ���� ���� Ȯ��
    ReadCPUID( 0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX );
    if( dwEDX & ( 1 << 29 ) )
    {
    	WriteLine( 45, 8, "Pass" );
    }
    else
    {
    	WriteLine( 45, 8, "Fail" );
    	WriteLine( 0, 9, "not support 64bit mode~!!" );
        while( 1 ) ;
    }

    CopyKernel64ImageTo2Mbyte();
    
    // IA-32e ���� ��ȯ
    WriteLine( 0, 10, "Switch To IA-32e Mode" );
    Change64BitKernel();
}

