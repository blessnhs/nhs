
#ifndef __LIST_H__
#define __LIST_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// �����͸� �����ϴ� �ڷᱸ��
// �ݵ�� �������� ���� �պκп� ��ġ�ؾ� ��
typedef struct kListLinkStruct
{
    // ���� �������� ��巹���� �����͸� �����ϱ� ���� ID
    void* pvNext;
    QWORD qwID;
} LISTLINK;

// ����Ʈ�� �����ϴ� �ڷᱸ��
typedef struct kListManagerStruct
{
    // ����Ʈ �������� ��
    int iItemCount;

    // ����Ʈ�� ù ��°�� ������ �������� ��巹��
    void* pvHeader;
    void* pvTail;
} LIST;

#pragma pack( pop )


////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void InitializeList( LIST* pstList );
int GetListCount( const LIST* pstList );
void AddListToTail( LIST* pstList, void* pvItem );
void AddListToHeader( LIST* pstList, void* pvItem );
void* RemoveList( LIST* pstList, QWORD qwID );
void* RemoveListFromHeader( LIST* pstList );
void* RemoveListFromTail( LIST* pstList );
void* FindList( const LIST* pstList, QWORD qwID );
void* GetHeaderFromList( const LIST* pstList );
void* GetTailFromList( const LIST* pstList );
void* GetNextFromList( const LIST* pstList, void* pstCurrent );

#endif /*__LIST_H__*/
