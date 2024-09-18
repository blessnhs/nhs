/**
 *  file    Utility.h
 *  date    2009/01/17
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   OS���� ����� ��ƿ��Ƽ �Լ��� ���õ� ����
 */

#include "Utility.h"
#include "AssemblyUtility.h"
#include <stdarg.h>
#include "VBE.h"
#include "../../32bit/source/page.h"

// PIT ��Ʈ�ѷ��� �߻��� Ƚ���� ������ ī����
volatile QWORD g_qwTickCount = 0;

/**
 *  �޸𸮸� Ư�� ������ ä��
 */


void *MemChr(const void *buf, int c, int n)
{
    unsigned char *p = (unsigned char *)buf;
    unsigned char *end = p + n;

    while (p != end)
    {
        if (*p == c)
        {
            return p;
        }

        ++p;
    }

    return 0;
}

void MemSet( void* pvDestination, BYTE bData, int iSize )
{
    int i;
    QWORD qwData;
    int iRemainByteStartOffset;
    
    // 8 ����Ʈ �����͸� ä��
    qwData = 0;
    for( i = 0 ; i < 8 ; i++ )
    {
        qwData = ( qwData << 8 ) | bData;
    }
    
    // 8 ����Ʈ�� ���� ä��
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = qwData;
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset++ ] = bData;
    }
}

/**
 *  �޸𸮸� 16bit Ư�� ������ ä��
 *      iSize�� ä�� 16bit �������� ������ �ǹ�
 */
inline void MemSetWord( void* pvDestination, WORD wData, int iWordSize )
{
    int i;
    QWORD qwData;
    int iRemainWordStartOffset;
    
    // 8 ����Ʈ�� WORD �����͸� ä��
    qwData = 0;
    for( i = 0 ; i < 4 ; i++ )
    {
        qwData = ( qwData << 16 ) | wData;
    }
    
    // 8 ����Ʈ�� ���� ä��, WORD �����͸� 4���� �Ѳ����� ä�� �� ����
    for( i = 0 ; i < ( iWordSize / 4 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = qwData;
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainWordStartOffset = i * 4;
    for( i = 0 ; i < ( iWordSize % 4 ) ; i++ )
    {
        ( ( WORD* ) pvDestination )[ iRemainWordStartOffset++ ] = wData;
    }
}


/*
void MemSet( void* pvDestination, BYTE bData, int iSize )
{
    int i;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        ( ( char* ) pvDestination )[ i ] = bData;
    }
}
*/

/**
 *  �޸� ����
 */
int MemCpy( void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    int iRemainByteStartOffset;
    
    // 8 ����Ʈ�� ���� ����
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        ( ( QWORD* ) pvDestination )[ i ] = ( ( QWORD* ) pvSource )[ i ];
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        ( ( char* ) pvDestination )[ iRemainByteStartOffset ] = 
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        iRemainByteStartOffset++;
    }
    return iSize;
}

int32_t strcmp(const char* s1, const char* s2)
{
    while ((*s1) && (*s1 == *s2))
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

int strncmp (const char *cs, const char *ct, size_t count)
{
	signed char res = 0;

	while (count) {
		if ((res = *cs - *ct ++) || !*cs ++)
			break;

		count --;
	}

	return res;
}

int memcpy( void* pvDestination, const void* pvSource, int iSize )
{
	MemCpy(pvDestination,pvSource,iSize);
}

/*
int MemCpy( void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        ( ( char* ) pvDestination )[ i ] = ( ( char* ) pvSource )[ i ];
    }
    
    return iSize;
}
*/

/**
 *  �޸� ��
 */
int MemCmp( const void* pvDestination, const void* pvSource, int iSize )
{
    int i, j;
    int iRemainByteStartOffset;
    QWORD qwValue;
    char cValue;
    
    // 8 ����Ʈ�� ���� ��
    for( i = 0 ; i < ( iSize / 8 ) ; i++ )
    {
        qwValue = ( ( QWORD* ) pvDestination )[ i ] - ( ( QWORD* ) pvSource )[ i ];

        // Ʋ�� ��ġ�� ��Ȯ�ϰ� ã�Ƽ� �� ���� ��ȯ
        if( qwValue != 0 )
        {
            for( i = 0 ; i < 8 ; i++ )
            {
                if( ( ( qwValue >> ( i * 8 ) ) & 0xFF ) != 0 )
                {
                    return ( qwValue >> ( i * 8 ) ) & 0xFF;
                }
            }
        }
    }
    
    // 8 ����Ʈ�� ä��� ���� �κ��� ������
    iRemainByteStartOffset = i * 8;
    for( i = 0 ; i < ( iSize % 8 ) ; i++ )
    {
        cValue = ( ( char* ) pvDestination )[ iRemainByteStartOffset ] -
            ( ( char* ) pvSource )[ iRemainByteStartOffset ];
        if( cValue != 0 )
        {
            return cValue;
        }
        iRemainByteStartOffset++;
    }    
    return 0;
}
/*
int MemCmp( const void* pvDestination, const void* pvSource, int iSize )
{
    int i;
    char cTemp;
    
    for( i = 0 ; i < iSize ; i++ )
    {
        cTemp = ( ( char* ) pvDestination )[ i ] - ( ( char* ) pvSource )[ i ];
        if( cTemp != 0 )
        {
            return ( int ) cTemp;
        }
    }
    return 0;
}
*/

/**
 *  RFLAGS ���������� ���ͷ�Ʈ �÷��׸� �����ϰ� ���� ���ͷ�Ʈ �÷����� ���¸� ��ȯ
 */
BOOL SetInterruptFlag( BOOL bEnableInterrupt )
{
    QWORD qwRFLAGS;
    
    // ������ RFLAGS �������� ���� ���� �ڿ� ���ͷ�Ʈ ����/�Ұ� ó��
    qwRFLAGS = ReadRFLAGS();
    if( bEnableInterrupt == TRUE )
    {
        EnableInterrupt();
    }
    else
    {
        DisableInterrupt();
    }
    
    // ���� RFLAGS ���������� IF ��Ʈ(��Ʈ 9)�� Ȯ���Ͽ� ������ ���ͷ�Ʈ ���¸� ��ȯ
    if( qwRFLAGS & 0x0200 )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  ���ڿ��� ���̸� ��ȯ
 */
int kStrLen( const char* pcBuffer )
{
    int i;
    
    for( i = 0 ; ; i++ )
    {
        if( pcBuffer[ i ] == '\0' )
        {
            break;
        }
    }
    return i;
}

// ���� �� ũ��(Mbyte ����)
static gs_qwTotalRAMMBSize = 0;

/**
 *  64Mbyte �̻��� ��ġ���� �� ũ�⸦ üũ
 *      ���� ���� �������� �ѹ��� ȣ���ؾ� ��
 */
void CheckTotalRAMSize( void )
{
    DWORD* pdwCurrentAddress;
    DWORD dwPreviousValue;
    
    // 64Mbyte(0x4000000)���� 4Mbyte������ �˻� ����
    pdwCurrentAddress = ( DWORD* ) 0x4000000;
    while( 1 )
    {
        // ������ �޸𸮿� �ִ� ���� ����
        dwPreviousValue = *pdwCurrentAddress;
        // 0x12345678�� �Ἥ �о��� �� ������ ���� �������� ��ȿ�� �޸� 
        // �������� ����
        *pdwCurrentAddress = 0x12345678;
        if( *pdwCurrentAddress != 0x12345678 )
        {
            break;
        }
        // ���� �޸� ������ ����
        *pdwCurrentAddress = dwPreviousValue;
        // ���� 4Mbyte ��ġ�� �̵�
        pdwCurrentAddress += ( 0x400000 / 4 );
    }
    // üũ�� ������ ��巹���� 1Mbyte�� ������ Mbyte ������ ���
    gs_qwTotalRAMMBSize = ( QWORD ) pdwCurrentAddress / 0x100000;
}   

/**
 *  RAM ũ�⸦ ��ȯ
 */
QWORD GetTotalRAMSize( void )
{
    return gs_qwTotalRAMMBSize;
}

/**
 *  atoi() �Լ��� ���� ����
 */
long AToI( const char* pcBuffer, int iRadix )
{
    long lReturn;
    
    switch( iRadix )
    {
        // 16����
    case 16:
        lReturn = HexStringToQword( pcBuffer );
        break;
        
        // 10���� �Ǵ� ��Ÿ
    case 10:
    default:
        lReturn = DecimalStringToLong( pcBuffer );
        break;
    }
    return lReturn;
}

/**
 *  16���� ���ڿ��� QWORD�� ��ȯ 
 */
QWORD HexStringToQword( const char* pcBuffer )
{
    QWORD qwValue = 0;
    int i;
    
    // ���ڿ��� ���鼭 ���ʷ� ��ȯ
    for( i = 0 ; pcBuffer[ i ] != '\0' ; i++ )
    {
        qwValue *= 16;
        if( ( 'A' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'Z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'A' ) + 10;
        }
        else if( ( 'a' <= pcBuffer[ i ] )  && ( pcBuffer[ i ] <= 'z' ) )
        {
            qwValue += ( pcBuffer[ i ] - 'a' ) + 10;
        }
        else 
        {
            qwValue += pcBuffer[ i ] - '0';
        }
    }
    return qwValue;
}

/**
 *  10���� ���ڿ��� long���� ��ȯ
 */
long DecimalStringToLong( const char* pcBuffer )
{
    long lValue = 0;
    int i;
    
    // �����̸� -�� �����ϰ� �������� ���� long���� ��ȯ
    if( pcBuffer[ 0 ] == '-' )
    {
        i = 1;
    }
    else
    {
        i = 0;
    }
    
    // ���ڿ��� ���鼭 ���ʷ� ��ȯ
    for( ; pcBuffer[ i ] != '\0' ; i++ )
    {
        lValue *= 10;
        lValue += pcBuffer[ i ] - '0';
    }
    
    // �����̸� - �߰�
    if( pcBuffer[ 0 ] == '-' )
    {
        lValue = -lValue;
    }
    return lValue;
}

/**
 *  itoa() �Լ��� ���� ����
 */
int IToA( long lValue, char* pcBuffer, int iRadix )
{
    int iReturn;
    
    switch( iRadix )
    {
        // 16����
    case 16:
        iReturn = HexToString( lValue, pcBuffer );
        break;
        
        // 10���� �Ǵ� ��Ÿ
    case 10:
    default:
        iReturn = DecimalToString( lValue, pcBuffer );
        break;
    }
    
    return iReturn;
}

/**
 *  16���� ���� ���ڿ��� ��ȯ
 */
int HexToString( QWORD qwValue, char* pcBuffer )
{
    QWORD i;
    QWORD qwCurrentValue;

    // 0�� ������ �ٷ� ó��
    if( qwValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // ���ۿ� 1�� �ڸ����� 16, 256, ...�� �ڸ� ������ ���� ����
    for( i = 0 ; qwValue > 0 ; i++ )
    {
        qwCurrentValue = qwValue % 16;
        if( qwCurrentValue >= 10 )
        {
            pcBuffer[ i ] = 'A' + ( qwCurrentValue - 10 );
        }
        else
        {
            pcBuffer[ i ] = '0' + qwCurrentValue;
        }
        
        qwValue = qwValue / 16;
    }
    pcBuffer[ i ] = '\0';
    
    // ���ۿ� ����ִ� ���ڿ��� ����� ... 256, 16, 1�� �ڸ� ������ ����
    ReverseString( pcBuffer );
    return i;
}

/**
 *  10���� ���� ���ڿ��� ��ȯ
 */
int DecimalToString( long lValue, char* pcBuffer )
{
    long i;

    // 0�� ������ �ٷ� ó��
    if( lValue == 0 )
    {
        pcBuffer[ 0 ] = '0';
        pcBuffer[ 1 ] = '\0';
        return 1;
    }
    
    // ���� �����̸� ��� ���ۿ� '-'�� �߰��ϰ� ����� ��ȯ
    if( lValue < 0 )
    {
        i = 1;
        pcBuffer[ 0 ] = '-';
        lValue = -lValue;
    }
    else
    {
        i = 0;
    }

    // ���ۿ� 1�� �ڸ����� 10, 100, 1000 ...�� �ڸ� ������ ���� ����
    for( ; lValue > 0 ; i++ )
    {
        pcBuffer[ i ] = '0' + lValue % 10;        
        lValue = lValue / 10;
    }
    pcBuffer[ i ] = '\0';
    
    // ���ۿ� ����ִ� ���ڿ��� ����� ... 1000, 100, 10, 1�� �ڸ� ������ ����
    if( pcBuffer[ 0 ] == '-' )
    {
        // ������ ���� ��ȣ�� �����ϰ� ���ڿ��� ������
        ReverseString( &( pcBuffer[ 1 ] ) );
    }
    else
    {
        ReverseString( pcBuffer );
    }
    
    return i;
}

/**
 *  ���ڿ��� ������ ������
 */
void ReverseString( char* pcBuffer )
{
   int iLength;
   int i;
   char cTemp;
   
   
   // ���ڿ��� ����� �߽����� ��/�츦 �ٲ㼭 ������ ������
   iLength = kStrLen( pcBuffer );
   for( i = 0 ; i < iLength / 2 ; i++ )
   {
       cTemp = pcBuffer[ i ];
       pcBuffer[ i ] = pcBuffer[ iLength - 1 - i ];
       pcBuffer[ iLength - 1 - i ] = cTemp;
   }
}

/**
 *  sPrintf() �Լ��� ���� ����
 */
int SPrintf( char* pcBuffer, const char* pcFormatString, ... )
{
    va_list ap;
    int iReturn;
    
    // ���� ���ڸ� ������ vsPrintf() �Լ��� �Ѱ���
    va_start( ap, pcFormatString );
    iReturn = VSPrintf( pcBuffer, pcFormatString, ap );
    va_end( ap );
    
    return iReturn;
}

/**
 *  vsPrintf() �Լ��� ���� ����
 *      ���ۿ� ���� ���ڿ��� ���� �����͸� ����
 */
int VSPrintf( char* pcBuffer, const char* pcFormatString, va_list ap )
{
    QWORD i, j, k;
    int iBufferIndex = 0;
    int iFormatLength, iCopyLength;
    char* pcCopyString;
    QWORD qwValue;
    int iValue;
    double dValue;
    
    // ���� ���ڿ��� ���̸� �о ���ڿ��� ���̸�ŭ �����͸� ��� ���ۿ� ���
    iFormatLength = kStrLen( pcFormatString );
    for( i = 0 ; i < iFormatLength ; i++ ) 
    {
        // %�� �����ϸ� ������ Ÿ�� ���ڷ� ó��
        if( pcFormatString[ i ] == '%' ) 
        {
            // % ������ ���ڷ� �̵�
            i++;
            switch( pcFormatString[ i ] ) 
            {
                // ���ڿ� ���  
            case 's':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���ڿ� Ÿ������ ��ȯ
                pcCopyString = ( char* ) ( va_arg(ap, char* ));
                iCopyLength = kStrLen( pcCopyString );
                // ���ڿ��� ���̸�ŭ�� ��� ���۷� �����ϰ� ����� ���̸�ŭ 
                // ������ �ε����� �̵�
                MemCpy( pcBuffer + iBufferIndex, pcCopyString, iCopyLength );
                iBufferIndex += iCopyLength;
                break;
                
                // ���� ���
            case 'c':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ� 
                // ��� ���ۿ� �����ϰ� ������ �ε����� 1��ŭ �̵�
                pcBuffer[ iBufferIndex ] = ( char ) ( va_arg( ap, int ) );
                iBufferIndex++;
                break;

                // ���� ���
            case 'u':
            case 'd':
            case 'i':
                // ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                iValue = ( int ) ( va_arg( ap, int ) );
                iBufferIndex += IToA( iValue, pcBuffer + iBufferIndex, 10 );
                break;
                
                // 4����Ʈ Hex ���
            case 'x':
            case 'X':
                // ���� ���ڿ� ����ִ� �Ķ���͸� DWORD Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                qwValue = ( DWORD ) ( va_arg( ap, DWORD ) ) & 0xFFFFFFFF;
                iBufferIndex += IToA( qwValue, pcBuffer + iBufferIndex, 16 );
                break;

                // 8����Ʈ Hex ���
            case 'q':
            case 'Q':
            case 'p':
                // ���� ���ڿ� ����ִ� �Ķ���͸� QWORD Ÿ������ ��ȯ�Ͽ�
                // ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
                qwValue = ( QWORD ) ( va_arg( ap, QWORD ) );
                iBufferIndex += IToA( qwValue, pcBuffer + iBufferIndex, 16 );
                break;
            
                // �Ҽ��� ��° �ڸ����� �Ǽ��� ���
            case 'f':
                dValue = ( double) ( va_arg( ap, double ) );
                // ��° �ڸ����� �ݿø� ó��
                dValue += 0.005;
                // �Ҽ��� ��° �ڸ����� ���ʷ� �����Ͽ� ���۸� ������
                pcBuffer[ iBufferIndex ] = '0' + ( QWORD ) ( dValue * 100 ) % 10;
                pcBuffer[ iBufferIndex + 1 ] = '0' + ( QWORD ) ( dValue * 10 ) % 10;
                pcBuffer[ iBufferIndex + 2 ] = '.';
                for( k = 0 ; ; k++ )
                {
                    // ���� �κ��� 0�̸� ����
                    if( ( ( QWORD ) dValue == 0 ) && ( k != 0 ) )
                    {
                        break;
                    }
                    pcBuffer[ iBufferIndex + 3 + k ] = '0' + ( ( QWORD ) dValue % 10 );
                    dValue = dValue / 10;
                }
                pcBuffer[ iBufferIndex + 3 + k ] = '\0';
                // ���� ����� ���̸�ŭ ������ ���̸� ������Ŵ
                ReverseString( pcBuffer + iBufferIndex );
                iBufferIndex += 3 + k;
                break;
                // ���� �ش����� ������ ���ڸ� �״�� ����ϰ� ������ �ε�����
                // 1��ŭ �̵�
            default:
                pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
                iBufferIndex++;
                break;
            }
        } 
        // �Ϲ� ���ڿ� ó��
        else
        {
            // ���ڸ� �״�� ����ϰ� ������ �ε����� 1��ŭ �̵�
            pcBuffer[ iBufferIndex ] = pcFormatString[ i ];
            iBufferIndex++;
        }
    }
    
    // NULL�� �߰��Ͽ� ������ ���ڿ��� ����� ����� ������ ���̸� ��ȯ
    pcBuffer[ iBufferIndex ] = '\0';
    return iBufferIndex;
}

/**
 *  Tick Count�� ��ȯ
 */
QWORD GetTickCount( void )
{
    return g_qwTickCount;
}

/**
 *  �и�������(milisecond) ���� ���
 */
void Sleep( QWORD qwMillisecond )
{
    QWORD qwLastTickCount;
    
    qwLastTickCount = g_qwTickCount;
    
    while( ( g_qwTickCount - qwLastTickCount ) <= qwMillisecond )
    {
        Schedule();
    }
}

/**
 *  �׷��� ������� ���θ� ��ȯ
 */
BOOL IsGraphicMode( void )
{
    // �׷��� ��� ���� ���ΰ� ����� ��巹��(0x7C0A)
    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 )
    {
        return FALSE;
    }
    
    return TRUE;
}


unsigned long long *GetEntryLevel(unsigned long long address, int level)
{
	unsigned long long index = address / 0x1000;
	unsigned long long indices[4] = {
        index % 512,
        (index >> 9) % 512,
        (index >> 18) % 512,
        (index >> 27) % 512
    };
	unsigned long long *latest = (unsigned long long *)ReadCr3();

	int l;
    for(l = 3; l >= level; l --) {
    	unsigned long long *followed = (unsigned long long *)(*latest & ~0xfff);
        // if the present bit (P) is clear and we have more to go, then error.
        if(l != level && (*latest & 1) == 0) {
            return 0;
        }
        // TODO: check for size bit here
        latest = followed + indices[l];
    }
    return *latest;
}

unsigned long long *GetPhysAddr(void *virtualaddr)
{
	unsigned long pml4index     = (unsigned long)virtualaddr >> 39 & 0x1ff;
	unsigned long pdpindex 		= (unsigned long)virtualaddr >> 30 & 0x1ff;
	unsigned long pdindex  		= (unsigned long)virtualaddr >> 21 & 0x1ff;
//	unsigned long ptindex  		= (unsigned long)virtualaddr >> 12 & 0x1ff;
//	unsigned long offset   		= (unsigned long)virtualaddr & 0xfff;

	//4�ܰ������ ��� (pml4,pdp,pd)

//	Printf("pml4 %d pdpindex %d pdindex %d \n",	pml4index,pdpindex,pdindex);

	PML4TENTRY *pstPML4TEntry = ( PML4TENTRY* ) ReadCr3();
//	PDPTENTRY  *pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
//	PDENTRY    *pstPDEntry = ( PDENTRY* ) 0x102000;

	unsigned long lowAddress = ((pstPML4TEntry[pml4index].dwAttributeAndLowerBaseAddress >> 12));
	unsigned long upperAddress = (pstPML4TEntry[pml4index].dwUpperBaseAddressAndEXB & 0xff);

	unsigned long long pdp4Address = lowAddress;
	pdp4Address = pdp4Address << 12 | upperAddress;
//	Printf("lowAddress %q upperAddress %q pdpe Address %q base %q\n",lowAddress,upperAddress,pdp4Address,pstPML4TEntry->dwAttributeAndLowerBaseAddress);

	PDPTENTRY *pstPDPTEntry = ( PDPTENTRY* ) pdp4Address;

	lowAddress = ((pstPDPTEntry[pdpindex].dwAttributeAndLowerBaseAddress >> 12));
	upperAddress = (pstPDPTEntry[pdpindex].dwUpperBaseAddressAndEXB & 0xff);
	unsigned long long pdAddress = lowAddress;
	pdAddress = pdAddress << 12 | upperAddress;

//	Printf("lowAddress %q upperAddress %q pde Address %q base %q\n",lowAddress,upperAddress,pdAddress,pstPDPTEntry->dwAttributeAndLowerBaseAddress);
	PDENTRY    *pstPDEntry = ( PDENTRY* ) pdAddress;

	lowAddress = ((pstPDEntry[pdindex].dwAttributeAndLowerBaseAddress >> 12));
	upperAddress = (pstPDEntry[pdindex].dwUpperBaseAddressAndEXB & 0xff);
	unsigned long long pAddress = lowAddress;
	pdAddress = pdAddress << 12 | upperAddress;


//	Printf("pdentry %q %q\n",virtualaddr,pdAddress);

	unsigned long virtualoffset = ((unsigned long )virtualaddr & 0x1FFFFF);

	unsigned long long *ptr = (unsigned long long *)pdAddress + virtualoffset;

	return ptr;//(unsigned long long *)(&pstPDEntry[pdindex] + ((unsigned long long *)virtualaddr & (unsigned long long *)0x1FFFFF));
}

size_t strlen(const char* str)
{
    size_t retval = 0;
    for (; *str != '\0'; ++str)
        ++retval;
    return retval;
}

int bsr(int val)
{
  int result;
  __asm__("bsr %1, %0" : "=r"(result) : "r"(val));
  return result;
}


uint32_t alignUp(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    --alignment;
    return (val+alignment) & ~alignment;
}


uint32_t alignDown(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    return val & ~(alignment-1);
}

uint32_t* MemSetl(uint32_t* dest, uint32_t val, size_t dwords)
{
    uint32_t* retval = dest;
    __asm__ volatile("cld\n"
                     "rep stosl" : "+D"(dest), "+c"(dwords) : "a"(val) : "memory");

    return retval;
}

void* memset(void* dest, char val, size_t bytes)
{
    void* retval = dest;
    uint8_t uval = val;
    size_t dwords = bytes/4; // Number of dwords (4 Byte blocks) to be written
    bytes %= 4;              // Remaining bytes
    uint32_t dval = (uval<<24)|(uval<<16)|(uval<<8)|uval; // Create dword from byte value
    __asm__ volatile("cld\n"
                     "rep stosl\n"
                     "mov %%edx, %%ecx\n"
                     "rep stosb" : "+D"(dest), "+c"(dwords) : "a"(dval), "d"(bytes) : "memory");
    return retval;
}

uint16_t htons(uint16_t v)
{
    __asm__("xchg %h0, %b0" : "+Q"(v));
    return (v);
}

uint32_t htonl(uint32_t v)
{
    __asm__("bswap %0" : "+r"(v));
    return (v);
}

char* strcat(char* dest, const char* src)
{
    strcpy(dest + strlen(dest), src);
    return dest;
}

char* strcpy(char* dest, const char* src)
{
    char* save = dest;
    while ((*dest++ = *src++));
    return save;
}

char* strchr(const char* str, int character)
{
    for (;; str++)
    {
        // the order here is important
        if (*str == character)
        {
            return (char*)str;
        }
        if (*str == 0) // end of string
        {
            return (0);
        }
    }
}

int isspace(int c)
{
    return ((c >= 0x09 && c <= 0x0D) || c == ' ');
}


long int strtol(const char* nptr, char** endptr, int base)
{
    long num = 0;

    if (base && (base < 2 || base > 36))
        return num;

    while (isspace(*nptr))
        ++nptr; // skip spaces

    int sign = *nptr == '-' ? -1 : 1;
    if (*nptr == '-' || *nptr == '+')
        ++nptr;

    if (base == 0)
    {
        base = 10;
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
        {
            ++nptr;
            base = 16;
        }
    }
    else if (base == 16)
    {
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
            ++nptr;
    }

    while (*nptr == '0')
        ++nptr;

    for (; *nptr != '\0'; ++nptr)
    {
        if (*nptr - '0' < base && *nptr - '0' >= 0)
        {
            num = num * base + *nptr - '0';
            if (num <= 0)
            {
                num = -1;
                break;
            }
        }
        else if ((*nptr | 0x20) - 'a' + 10 < base &&
            (*nptr | 0x20) - 'a' + 10 >= 0)
        { // 'x' | 0x20 =~= tolower('X')
            num = num * base + (*nptr | 0x20) - 'a' + 10;
            if (num <= 0)
            {
                num = -1;
                break;
            }
        }
        else
        {
            break;
        }
    }

    for (; *nptr != '\0'; ++nptr)
    {
        // skip rest of integer constant
        if (!(*nptr - '0' < base && *nptr - '0' >= 0) &&
                !((*nptr | 0x20) - 'a' + 10 < base) &&
                !((*nptr | 0x20) - 'a' + 10 >= 0))
        {
            break;
        }
    }

    if (endptr)
        *endptr = (char*)nptr;

    return num * sign;
}

int atoi(const char* nptr)
{
    return (int)strtol(nptr, 0, 10);
}


void memshow(const void* start, size_t count, bool alpha)
{
	size_t i = 0;
    for (i = 0; i < count; i++)
    {
        if (alpha)
        {
            Printf("%c",((const char*)start)[i]);
        }
        else
        {
            if (i%16 == 0)
            	 Printf("\n");
            Printf("%d ", ((const uint8_t*)start)[i]);
        }
    }
}

char *strstr(char *string, char *substring)
{
    char *a, *b;
    int idx = 0;
    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */

    b = substring;
    if (*b == 0)
    {
    	return string;
    }

    for ( ; *string != 0; string += 1)
    {
    	if (*string != *b)
    	{
    		continue;
    	}

    	a = string;

		while (1)
		{
			if (*b == 0)
			{
				return string;
			}
			if (*a++ != *b++)
			{
				break;
			}
		}

		b = substring;
    }
    return (char *) 0;
}

