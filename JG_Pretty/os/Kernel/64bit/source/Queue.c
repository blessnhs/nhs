/**
 *  file    Queue.h
 *  date    2009/01/25
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ť�� ���õ� �Լ��� ������ �ҽ� ����
 */

#include "Queue.h"

/**
 *  ť�� �ʱ�ȭ
 */
void InitializeQueue( QUEUE* pstQueue, void* pvQueueBuffer, int iMaxDataCount, 
		int iDataSize )
{
    // ť�� �ִ� ������ ũ��, �׸��� ���� ��巹���� ����
	pstQueue->iMaxDataCount = iMaxDataCount;
	pstQueue->iDataSize = iDataSize;
	pstQueue->pvQueueArray = pvQueueBuffer;

    // ť�� ���� ��ġ�� ���� ��ġ�� �ʱ�ȭ�ϰ� ���������� ����� ����� ���ŷ�
    // �����Ͽ� ť�� �� ���·� ����
	pstQueue->iPutIndex = 0;
	pstQueue->iGetIndex = 0;
	pstQueue->bLastOperationPut = FALSE;
}

/**
 *  ť�� ���� á���� ���θ� ��ȯ
 */
BOOL IsQueueFull( const QUEUE* pstQueue )
{
    // ť�� ���� �ε����� ���� �ε����� ���� ���������� ����� ����� �����̸�
    // ť�� ���� á���Ƿ� ������ �� ����
    if( ( pstQueue->iGetIndex == pstQueue->iPutIndex ) &&
        ( pstQueue->bLastOperationPut == TRUE ) )
    {
        return TRUE;
    }
    return FALSE;
}

/**
 *  ť�� ������� ���θ� ��ȯ
 */
BOOL IsQueueEmpty( const QUEUE* pstQueue )
{
    // ť�� ���� �ε����� ���� �ε����� ���� ���������� ����� ����� �����̸�
    // ť�� ������Ƿ� ������ �� ����
    if( ( pstQueue->iGetIndex == pstQueue->iPutIndex ) &&
        ( pstQueue->bLastOperationPut == FALSE ) )
    {
        return TRUE;
    }
    return FALSE;
}   

/**
 * 	ť�� �����͸� ����
 */
BOOL PutQueue( QUEUE* pstQueue, const void* pvData )
{
    // ť�� ���� á���� ������ �� ����
	if( IsQueueFull( pstQueue ) == TRUE )
	{
	    return FALSE;
	}
	
	// ���� �ε����� ����Ű�� ��ġ���� �������� ũ�⸸ŭ�� ����
	MemCpy( ( char* ) pstQueue->pvQueueArray + ( pstQueue->iDataSize * 
			pstQueue->iPutIndex ), pvData, pstQueue->iDataSize );
	
    // ���� �ε����� �����ϰ� ���� ������ ���������� ���
	pstQueue->iPutIndex = ( pstQueue->iPutIndex + 1 ) % pstQueue->iMaxDataCount;
	pstQueue->bLastOperationPut = TRUE;
	return TRUE;
}

/**
 * 	ť���� �����͸� ����
 */
BOOL GetQueue( QUEUE* pstQueue, void* pvData )
{
    // ť�� ������� ������ �� ����
    if( IsQueueEmpty( pstQueue ) == TRUE )
    {
        return FALSE;
    }
	
	// ���� �ε����� ����Ű�� ��ġ���� �������� ũ�⸸ŭ�� ����
	MemCpy( pvData, ( char* ) pstQueue->pvQueueArray + ( pstQueue->iDataSize * 
			 pstQueue->iGetIndex ), pstQueue->iDataSize );
	
    // ���� �ε����� �����ϰ� ���� ������ ���������� ���
	pstQueue->iGetIndex = ( pstQueue->iGetIndex + 1 ) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = FALSE;
	return TRUE;
}
