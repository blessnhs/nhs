/**
 *  file    List.h
 *  date    2009/02/23
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui 
 *  brief   ����Ʈ�� ���õ� �Լ��� ������ �ҽ� ����
 */

#include "List.h"

/**
 *  ����Ʈ�� �ʱ�ȭ
 */
void InitializeList( LIST* pstList )
{
    pstList->iItemCount = 0;
    pstList->pvHeader = NULL;
    pstList->pvTail = NULL;
}

/**
 *  ����Ʈ�� ���Ե� �������� ���� ��ȯ
 */
int GetListCount( const LIST* pstList )
{
    return pstList->iItemCount;
}

/**
 *  ����Ʈ�� �����͸� ����
 */
void AddListToTail( LIST* pstList, void* pvItem )
{
    LISTLINK* pstLink;
    
    // ���� �������� ��巹���� ����(NULL)���� ����
    pstLink = ( LISTLINK* ) pvItem;
    pstLink->pvNext = NULL;
    
    // ����Ʈ�� �� �����̸� Header�� Tail�� �߰��� �����ͷ� ����
    if( pstList->pvHeader == NULL )
    {
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return ;
    }
    
    // ������ �������� LISTLINK�� ��ġ�� ���Ͽ� ���� �����͸� �߰��� �����ͷ� ����
    pstLink = ( LISTLINK* ) pstList->pvTail;
    pstLink->pvNext = pvItem;

    // ����Ʈ�� ������ �����͸� �߰��� �����ͷ� ����
    pstList->pvTail = pvItem;
    pstList->iItemCount++;
}

/**
 *  ����Ʈ�� ù �κп� �����͸� ����
 */
void AddListToHeader( LIST* pstList, void* pvItem )
{
    LISTLINK* pstLink;
    
    // ���� �������� ��巹���� Header�� ����
    pstLink = ( LISTLINK* ) pvItem;
    pstLink->pvNext = pstList->pvHeader;    
    
    // ����Ʈ�� �� �����̸� Header�� Tail�� �߰��� �����ͷ� ����
    if( pstList->pvHeader == NULL )
    {
        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;
        
        return ;
    }
    
    // ����Ʈ�� ù ��° �����͸� �߰��� �����ͷ� ����
    pstList->pvHeader = pvItem;
    pstList->iItemCount++;
}

/**
 *  ����Ʈ���� �����͸� ������ ��, �������� �����͸� ��ȯ
 */
void* RemoveList( LIST* pstList, QWORD qwID )
{
    LISTLINK* pstLink;
    LISTLINK* pstPreviousLink;
    
    pstPreviousLink = ( LISTLINK* ) pstList->pvHeader;
    for( pstLink = pstPreviousLink ; pstLink != NULL ; pstLink = pstLink->pvNext )
    {
        // ��ġ�ϴ� ID�� �ִٸ� ����
        if( pstLink->qwID == qwID )
        {
            // ���� �����Ͱ� �ϳ� �ۿ� ���ٸ� ����Ʈ �ʱ�ȭ
            if( ( pstLink == pstList->pvHeader ) && 
                ( pstLink == pstList->pvTail ) )
            {
                pstList->pvHeader = NULL;
                pstList->pvTail = NULL;
            }
            // ���� ����Ʈ�� ù ��° �������̸� Header�� �� ��° �����ͷ� ����
            else if( pstLink == pstList->pvHeader )
            {
                pstList->pvHeader = pstLink->pvNext;
            }
            // ���� ����Ʈ�� ������ �����͸� Tail�� ������ ������ �����ͷ� ����
            else if( pstLink == pstList->pvTail )
            {
                pstList->pvTail = pstPreviousLink;
            }
            else
            {
                pstPreviousLink->pvNext = pstLink->pvNext;
            }
            
            pstList->iItemCount--;
            return pstLink;
        }
        pstPreviousLink = pstLink;
    }
    return NULL;
}

/**
 *  ����Ʈ�� ù ��° �����͸� �����Ͽ� ��ȯ
 */
void* RemoveListFromHeader( LIST* pstList )
{
    LISTLINK* pstLink;
    
    if( pstList->iItemCount == 0 )
    {
        return NULL;
    }

    // ����� �����ϰ�, ��ȯ
    pstLink = ( LISTLINK* ) pstList->pvHeader;
    return RemoveList( pstList, pstLink->qwID );
}

/**
 *  ����Ʈ�� ������ �����͸� �����Ͽ� ��ȯ
 */
void* RemoveListFromTail( LIST* pstList )
{
    LISTLINK* pstLink;
    
    if( pstList->iItemCount == 0 )
    {
        return NULL;
    }

    // ������ �����ϰ�, ��ȯ
    pstLink = ( LISTLINK* ) pstList->pvTail;
    return RemoveList( pstList, pstLink->qwID );
}


/**
 *  ����Ʈ���� �������� ã��
 */
void* FindList( const LIST* pstList, QWORD qwID )
{
    LISTLINK* pstLink;
    
    for( pstLink = ( LISTLINK* ) pstList->pvHeader ; pstLink != NULL ; 
         pstLink = pstLink->pvNext )
    {
        // ��ġ�ϴ� �� �ִٸ� ��ȯ
        if( pstLink->qwID == qwID )
        {
            return pstLink;
        }
    }
    return NULL;    
}

/**
 *  ����Ʈ�� ����� ��ȯ
 */
void* GetHeaderFromList( const LIST* pstList )
{
    return pstList->pvHeader;
}

/**
 *  ����Ʈ�� ������ ��ȯ
 */
void* GetTailFromList( const LIST* pstList )
{
    return pstList->pvTail;
}

/**
 *  ���� �������� ���� �������� ��ȯ 
 */
void* GetNextFromList( const LIST* pstList, void* pstCurrent )
{
    LISTLINK* pstLink;
    
    pstLink = ( LISTLINK* ) pstCurrent;

    return pstLink->pvNext;
}
