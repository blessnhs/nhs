/**
 *  file    Synchronization.h
 *  date    2009/03/13
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   ����ȭ�� ó���ϴ� �Լ��� ���õ� ����
 */

#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ���ؽ� �ڷᱸ��
typedef struct kMutexStruct
{
    // �½�ũ ID�� ����� ������ Ƚ��
    volatile QWORD qwTaskID;
    volatile DWORD dwLockCount;

    // ��� �÷���
    volatile BOOL bLockFlag;
    
    // �ڷᱸ���� ũ�⸦ 8����Ʈ ������ ���߷��� �߰��� �ʵ�
    BYTE vbPadding[ 3 ];
} MUTEX;

// ���ɶ� �ڷᱸ��
typedef struct kSpinLockStruct
{
    // ���� APIC ID�� ����� ������ Ƚ��
    volatile DWORD dwLockCount;
    volatile BYTE bAPICID;

    // ��� �÷���
    volatile BOOL bLockFlag;
    
    // ���ͷ�Ʈ �÷���
    volatile BOOL bInterruptFlag;
    
    // �ڷᱸ���� ũ�⸦ 8����Ʈ ������ ���߷��� �߰��� �ʵ�
    BYTE vbPadding[ 1 ];
} SPINLOCK;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
#if 0
BOOL LockForSystemData( void );
void UnlockForSystemData( BOOL bInterruptFlag );
#endif

void InitializeSpinLock( SPINLOCK* pstSpinLock );
void LockForSpinLock( SPINLOCK* pstSpinLock );
void UnlockForSpinLock( SPINLOCK* pstSpinLock );

void InitializeMutex( MUTEX* pstMutex );
void Lock( MUTEX* pstMutex );
void Unlock( MUTEX* pstMutex );

#define MUTEX_CREATE(name) MUTEX name = { .bLockFlag = 0,.dwLockCount = 0,.qwTaskID = 0xFFFFFFFFFFFFFFFF }

#endif /*__SYNCHRONIZATION_H__*/
