
#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"
#include "MultiProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//
//  ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���ͷ�Ʈ ������ �ִ� ����, ISA ������ ���ͷ�Ʈ�� ó���ϹǷ� 16
#define INTERRUPT_MAXVECTORCOUNT            16
// ���ͷ�Ʈ ���� �л��� �����ϴ� ����, ���ͷ�Ʈ ó�� Ƚ���� 10�� ����� �Ǵ� ����
#define INTERRUPT_LOADBALANCINGDIVIDOR      10

////////////////////////////////////////////////////////////////////////////////
//
//  ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ���ͷ�Ʈ�� ���õ� ������ �����ϴ� �ڷᱸ��
typedef struct kInterruptManagerStruct
{
    // �ھ� �� ���ͷ�Ʈ ó�� Ƚ��, �ִ� �ھ� ���� X �ִ� ���ͷ�Ʈ ���� ������ ���ǵ� 2���� �迭
    QWORD vvqwCoreInterruptCount[ MAXPROCESSORCOUNT ][ INTERRUPT_MAXVECTORCOUNT ];
    
    // ���� �л� ��� ��� ����
    BOOL bUseLoadBalancing;
    
    // ��Ī I/O ���(Symmetric I/O Mode) ��� ����
    BOOL bSymmetricIOMode;
} INTERRUPTMANAGER;


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void SetSymmetricIOMode( BOOL bSymmetricIOMode );
void SetInterruptLoadBalancing( BOOL bUseLoadBalancing );
void IncreaseInterruptCount( int iIRQ );
void SendEOI( int iIRQ );
INTERRUPTMANAGER* kGetInterruptManager( void );
void ProcessLoadBalancing( int iIRQ );

void CommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void CommonInterruptHandler( int iVectorNumber );
void KeyboardHandler( int iVectorNumber );
void TimerHandler( int iVectorNumber );
void DeviceNotAvailableHandler( int iVectorNumber );
void HDDHandler( int iVectorNumber );
void MouseHandler( int iVectorNumber );

#endif /*__INTERRUPTHANDLER_H__*/
