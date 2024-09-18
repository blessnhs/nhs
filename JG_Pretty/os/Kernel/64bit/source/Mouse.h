
#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "Types.h"
#include "Synchronization.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���콺 ť�� ���� ��ũ��
#define MOUSE_MAXQUEUECOUNT 100

// ��ư�� ���¸� ��Ÿ���� ��ũ��
#define MOUSE_LBUTTONDOWN   0x01
#define MOUSE_RBUTTONDOWN   0x02
#define MOUSE_MBUTTONDOWN   0x04

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// PS/2 ���콺 ��Ŷ�� �����ϴ� �ڷᱸ��, ���콺 ť�� �����ϴ� ������
typedef struct kMousePacketStruct
{
    // ��ư ���¿� X, Y ���� ���õ� �÷���
    BYTE bButtonStatusAndFlag;    
    // X�� �̵��Ÿ�
    BYTE bXMovement;    
    // Y�� �̵��Ÿ�
    BYTE bYMovement;

    BYTE bUSBMouse;
} MOUSEDATA;

#pragma pack( pop )

// ���콺�� ���¸� �����ϴ� �ڷᱸ��
typedef struct kMouseManagerStruct
{
    // �ڷᱸ�� ����ȭ�� ���� ���ɶ�
    SPINLOCK stSpinLock;    
    // ���� ���ŵ� �������� ����, ���콺 �����Ͱ� 3���̹Ƿ� 0~2�� ������ ��� �ݺ���
    int iByteCount;
    // ���� ���� ���� ���콺 ������
    MOUSEDATA stCurrentData;
} MOUSEMANAGER;

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL InitializeMouse( void );
BOOL AccumulateMouseDataAndPutQueue( BYTE bMouseData );

BOOL AccumulateMouseDataAndPutQueueUSB( BYTE x, BYTE y, BYTE status);

BOOL ActivateMouse( void );
void EnableMouseInterrupt( void );
BOOL IsMouseDataInOutputBuffer( void );
BOOL GetMouseDataFromMouseQueue( BYTE* pbButtonStatus, int* piRelativeX, 
        int* piRelativeY);

#endif /*__MOUSE_H__*/
