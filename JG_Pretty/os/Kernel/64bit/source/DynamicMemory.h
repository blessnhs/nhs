/**
 *  file    DynmaicMemory.h
 *  date    2009/04/11
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �޸� �Ҵ�� ������ ���õ� ��� ����
 */

#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"
#include "Synchronization.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �޸� ������ ���� ��巹��, 2Mbyte ������ ����
// �� �̻� Ŀ�� ������ ���� ������ �Ҵ����� �����Ƿ� Ŀ�� ���� Ǯ�� �����ϴ� ��ġ����
// ���� �޸� ������ �Ҵ�
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + 0x1fffff ) & \
                                          0xffffffffffe00000 )
// ���� ������ �ּ� ũ��, 1KB
#define DYNAMICMEMORY_MIN_SIZE          ( 1 * 1024 )

// ��Ʈ���� �÷���
#define DYNAMICMEMORY_EXIST             0x01
#define DYNAMICMEMORY_EMPTY             0x00

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ��Ʈ���� �����ϴ� �ڷᱸ��
typedef struct kBitmapStruct
{
    BYTE* pbBitmap;
    QWORD qwExistBitCount;
} BITMAP;

// ���� ������ �����ϴ� �ڷᱸ��
typedef struct kDynamicMemoryManagerStruct
{
    // �ڷᱸ�� ����ȭ�� ���� ���ɶ�
    SPINLOCK stSpinLock;
    
    // ���� ����Ʈ�� �� ������ ���� ũ�Ⱑ ���� ���� ������ ����, �׸��� �Ҵ�� �޸� ũ��
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;
    
    // ���� Ǯ�� ���� ��巹���� ������ ��巹��
    QWORD qwStartAddress;
    QWORD qwEndAddress;
    
    // �Ҵ�� �޸𸮰� ���� ���� ����Ʈ�� �ε����� �����ϴ� ������ ��Ʈ�� �ڷᱸ���� 
    // ��巹��
    BYTE* pbAllocatedBlockListIndex;
    BITMAP* pstBitmapOfLevel;
} DYNAMICMEMORY;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void InitializeDynamicMemory( void );
void* AllocateMemory( QWORD qwSize );
BOOL FreeMemory( void* pvAddress );
void GetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize ); 
DYNAMICMEMORY* kGetDynamicMemoryManager( void );

static QWORD kCalculateDynamicMemorySize( void );
static int CalculateMetaBlockCount( QWORD qwDynamicRAMSize );
static int AllocationBuddyBlock( QWORD qwAlignedSize );
static QWORD GetBuddyBlockSize( QWORD qwSize );
static int GetBlockListIndexOfMatchSize( QWORD qwAlignedSize );
static int FindFreeBlockInBitmap( int iBlockListIndex );
static void SetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag );
static BOOL FreeBuddyBlock( int iBlockListIndex, int iBlockOffset );
static BYTE GetFlagInBitmap( int iBlockListIndex, int iOffset );
void *krealloc (void *blk, size_t size);

#define NEW AllocateMemory
#define DEL FreeMemory
#endif /*__DYNAMICMEMORY_H__*/