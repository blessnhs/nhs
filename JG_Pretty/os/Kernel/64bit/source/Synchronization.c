/**
 *  file    Synchronization.c
 *  date    2009/03/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   ����ȭ�� ó���ϴ� �Լ��� ���õ� ����
 */

#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"
#include "AssemblyUtility.h"

// ���ɶ� ���� ������ ����ϴ� �Լ���
#if 0
/**
 *  �ý��� �������� ����ϴ� �����͸� ���� ��� �Լ�
 */
BOOL LockForSystemData( void )
{
    return SetInterruptFlag( FALSE );
}

/**
 *  �ý��� �������� ����ϴ� �����͸� ���� ��� ���� �Լ�
 */
void UnlockForSystemData( BOOL bInterruptFlag )
{
    SetInterruptFlag( bInterruptFlag );
}
#endif

/**
 *  ���ؽ��� �ʱ�ȭ 
 */
void InitializeMutex( MUTEX* pstMutex )
{
    // ��� �÷��׿� Ƚ��, �׸��� �½�ũ ID�� �ʱ�ȭ
    pstMutex->bLockFlag = FALSE;
    pstMutex->dwLockCount = 0;
    pstMutex->qwTaskID = TASK_INVALIDID;
}


/**
 *  �½�ũ ���̿��� ����ϴ� �����͸� ���� ��� �Լ�
 */
void Lock( MUTEX* pstMutex )
{
    BYTE bCurrentAPICID;
    BOOL bInterruptFlag;

    // ���ͷ�Ʈ�� ��Ȱ��ȭ
    bInterruptFlag = SetInterruptFlag( FALSE );

    // ���� �ھ��� ���� APIC ID�� Ȯ��
    bCurrentAPICID = GetAPICID();
    
    // �̹� ��� �ִٸ� ���� �ᰬ���� Ȯ���ϰ� ��� Ƚ���� ������Ų �� ����
    if( CMPXCHG(&( pstMutex->bLockFlag ), 0, 1 ) == FALSE )
    {
        // �ڽ��� �ᰬ�ٸ� Ƚ���� ������Ŵ
        if( pstMutex->qwTaskID == GetRunningTask( bCurrentAPICID )->stLink.qwID ) 
        {
            // ���ͷ�Ʈ�� ����
            SetInterruptFlag( bInterruptFlag );
            pstMutex->dwLockCount++;
            return ;
        }
        
        // �ڽ��� �ƴ� ���� ��� ���� ������ ������ ���
        while( CMPXCHG( &( pstMutex->bLockFlag ), 0, 1 ) == FALSE )
        {
            Schedule();
        }
    }
       
    // ��� ����, ��� �÷��״� ���� CMPXCHG() �Լ����� ó����
    pstMutex->dwLockCount = 1;
    pstMutex->qwTaskID = GetRunningTask( bCurrentAPICID )->stLink.qwID;
    // ���ͷ�Ʈ�� ����
    SetInterruptFlag( bInterruptFlag );
}

/**
 *  �½�ũ ���̿��� ����ϴ� �����͸� ���� ��� ���� �Լ�
 */
void Unlock( MUTEX* pstMutex )
{
    BOOL bInterruptFlag;

    // ���ͷ�Ʈ�� ��Ȱ��ȭ
    bInterruptFlag = SetInterruptFlag( FALSE );
    
    // ���ؽ��� ��� �½�ũ�� �ƴϸ� ����
    if( ( pstMutex->bLockFlag == FALSE ) ||
        ( pstMutex->qwTaskID != GetRunningTask( GetAPICID() )->stLink.qwID ) )
    {
        // ���ͷ�Ʈ�� ����
        SetInterruptFlag( bInterruptFlag );
        return ;
    }
    
    // ���ؽ��� �ߺ����� �ᰬ���� ��� Ƚ���� ����
    if( pstMutex->dwLockCount > 1 )
    {
        pstMutex->dwLockCount--;
    }
    else
    {
        // ������ ������ ����, ��� �÷��׸� ���� ���߿� �����ؾ� ��
        pstMutex->qwTaskID = TASK_INVALIDID;
        pstMutex->dwLockCount = 0;
        pstMutex->bLockFlag = FALSE;
    }
    // ���ͷ�Ʈ�� ����
    SetInterruptFlag( bInterruptFlag );
}

/**
 *  ���ɶ��� �ʱ�ȭ
 */
void InitializeSpinLock( SPINLOCK* pstSpinLock )
{
    // ��� �÷��׿� Ƚ��, APIC ID, ���ͷ�Ʈ �÷��׸� �ʱ�ȭ
    pstSpinLock->bLockFlag = FALSE;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bAPICID = 0xFF;
    pstSpinLock->bInterruptFlag = FALSE;
}

/**
 *  �ý��� �������� ����ϴ� �����͸� ���� ��� �Լ�
 */
void LockForSpinLock( SPINLOCK* pstSpinLock )
{
    BOOL bInterruptFlag;
    
    // ���ͷ�Ʈ�� ���� ��Ȱ��ȭ
    bInterruptFlag = SetInterruptFlag( FALSE );

    // �̹� ��� �ִٸ� ���� �ᰬ���� Ȯ���ϰ� �׷��ٸ� ��� Ƚ���� ������Ų �� ����
    if( CMPXCHG(&( pstSpinLock->bLockFlag ), 0, 1 ) == FALSE )
    {
        // �ڽ��� �ᰬ�ٸ� Ƚ���� ������Ŵ
        if( pstSpinLock->bAPICID == GetAPICID() )
        {
            pstSpinLock->dwLockCount++;
            return ;
        }
        
        // �ڽ��� �ƴ� ���� ��� ���� ������ ������ ���
        while( CMPXCHG( &( pstSpinLock->bLockFlag ), 0, 1 ) == FALSE )
        {
            // CMPXCHG() �Լ��� ��� ȣ���Ͽ� �޸� ������ Lock �Ǵ� ���� ����
            while( pstSpinLock->bLockFlag == TRUE )
            {
                Pause();
            }
        }
    }
    
    // ��� ����, ��� �÷��״� ���� CMPXCHG() �Լ����� ó����
    pstSpinLock->dwLockCount = 1;
    pstSpinLock->bAPICID = GetAPICID();

    // ���ͷ�Ʈ �÷��׸� �����Ͽ� Unlock ���� �� ����
    pstSpinLock->bInterruptFlag = bInterruptFlag;
}

/**
 *  �ý��� �������� ����ϴ� �����͸� ���� ��� ���� �Լ�
 */
void UnlockForSpinLock( SPINLOCK* pstSpinLock )
{
    BOOL bInterruptFlag;
    
    // ���ͷ�Ʈ�� ���� ��Ȱ��ȭ
    bInterruptFlag = SetInterruptFlag( FALSE );

    // ���ɶ��� ��� �½�ũ�� �ƴϸ� ����
    if( ( pstSpinLock->bLockFlag == FALSE ) ||
        ( pstSpinLock->bAPICID != GetAPICID() ) )
    {
        SetInterruptFlag( bInterruptFlag );
        return ;
    }
    
    // ���ɶ��� �ߺ����� �ᰬ���� ��� Ƚ���� ����
    if( pstSpinLock->dwLockCount > 1 )
    {
        pstSpinLock->dwLockCount--;
        return ;
    }
    
    // ���ɶ��� ������ ������ �����ϰ� ���ͷ�Ʈ �÷��׸� ����
    // ���ͷ�Ʈ �÷��״� �̸� �����صξ��ٰ� ���
    bInterruptFlag = pstSpinLock->bInterruptFlag;    
    pstSpinLock->bAPICID = 0xFF;
    pstSpinLock->dwLockCount = 0;
    pstSpinLock->bInterruptFlag = FALSE;
    pstSpinLock->bLockFlag = FALSE;  
    
    SetInterruptFlag( bInterruptFlag );
}
