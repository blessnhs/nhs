/**
 *  file    Queue.h
 *  date    2009/01/25
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ť�� ���õ� �Լ��� ������ ��� ����
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ť�� ���� ����ü
typedef struct kQueueManagerStruct
{
    // ť�� �����ϴ� ������ �ϳ��� ũ��� �ִ� ����
    int iDataSize;
    int iMaxDataCount;

    // ť ������ �����Ϳ� ����/���� �ε���
    void* pvQueueArray;
    int iPutIndex;
    int iGetIndex;
    
    // ť�� ����� ������ ����� ���������� ����
    BOOL bLastOperationPut;
} QUEUE;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void InitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, 
		int iDataSize );
BOOL IsQueueFull( const QUEUE* pstQueue );
BOOL IsQueueEmpty( const QUEUE* pstQueue );
BOOL PutQueue( QUEUE* pstQueue, const void* pvData );
BOOL GetQueue( QUEUE* pstQueue, void* pvData );

#endif /*__QUEUE_H__*/
