/**
 *  file    2DGraphics.h
 *  date    2009/09/5
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   2D Graphic�� ���� �ҽ� ����
 */

#include "Graphics.h"
#include "VBE.h"
#include "Font.h"
#include "Utility.h"

/**
 *  (x, y)�� �簢�� ���� �ȿ� �ִ��� ���θ� ��ȯ
 */
inline BOOL IsInRectangle( const RECT* pstArea, int iX, int iY )
{
    // ȭ�鿡 ǥ�õǴ� ������ ����ٸ� �׸��� ����
    if( ( iX <  pstArea->iX1 ) || ( pstArea->iX2 < iX ) ||
        ( iY <  pstArea->iY1 ) || ( pstArea->iY2 < iY ) )
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 *  �簢���� �ʺ� ��ȯ
 */
inline int GetRectangleWidth( const RECT* pstArea )
{
    int iWidth;
    
    iWidth = pstArea->iX2 - pstArea->iX1 + 1;
    
    if( iWidth < 0 )
    {
        return -iWidth;
    }
    
    return iWidth;
}

/**
 *  �簢���� ���̸� ��ȯ
 */
inline int GetRectangleHeight( const RECT* pstArea )
{
    int iHeight;
    
    iHeight = pstArea->iY2 - pstArea->iY1 + 1;
    
    if( iHeight < 0 )
    {
        return -iHeight;
    }
    
    return iHeight;
}

/**
 *  �� ���� �簢���� �����ϴ°� �Ǵ��Ͽ� ����� ��ȯ
 */
inline BOOL IsRectangleOverlapped( const RECT* pstArea1, const RECT* pstArea2 )
{
    // ���� 1�� ������ ���� 2�� ���������� ���� ��쳪
    // ���� 1�� �������� ���� 2�� �������� ū ���� ���� ��ġ�� �κ��� ����
    if( ( pstArea1->iX1 > pstArea2->iX2 ) || ( pstArea1->iX2 < pstArea2->iX1 ) ||
        ( pstArea1->iY1 > pstArea2->iY2 ) || ( pstArea1->iY2 < pstArea2->iY1 ) )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  ���� 1�� ���� 2�� ��ġ�� ������ ��ȯ
 */
inline BOOL GetOverlappedRectangle( const RECT* pstArea1, const RECT* pstArea2, 
        RECT* pstIntersection  )
{
    int iMaxX1;
    int iMinX2;
    int iMaxY1;
    int iMinY2;
    
    // X���� �������� �� �� �߿��� ū ���� ã��
    iMaxX1 = MAX( pstArea1->iX1, pstArea2->iX1 );
    // X���� ������ �� �� �߿��� ���� ���� ã��
    iMinX2 = MIN( pstArea1->iX2, pstArea2->iX2 );
    // ����� �������� ��ġ�� ������ ��ġ���� ũ�ٸ� �� �簢���� ��ġ�� ����
    if( iMinX2 < iMaxX1 )
    {
        return FALSE;
    }
    
    // Y���� �������� �� �� �߿��� ū ���� ã��
    iMaxY1 = MAX( pstArea1->iY1, pstArea2->iY1 );
    // Y���� ������ �� �� �߿��� ���� ���� ã��
    iMinY2 = MIN( pstArea1->iY2, pstArea2->iY2 );
    // ����� �������� ��ġ�� ������ ��ġ���� ũ�ٸ� �� �簢���� ��ġ�� ����
    if( iMinY2 < iMaxY1 )
    {
        return FALSE;
    }
    
    // ��ġ�� ������ ���� ����
    pstIntersection->iX1 = iMaxX1;
    pstIntersection->iY1 = iMaxY1;
    pstIntersection->iX2 = iMinX2;
    pstIntersection->iY2 = iMinY2;
    
    return TRUE;
}

/**
 *  �簢�� �ڷᱸ���� ä��
 *      x1�� x2, y1�� y2�� ���ؼ� x1 < x2, y1 < y2�� �ǵ��� ����
 */
inline void SetRectangleData( int iX1, int iY1, int iX2, int iY2, RECT* pstRect )
{
    // x1 < x2�� �ǵ��� RECT �ڷᱸ���� X��ǥ�� ����
    if( iX1 < iX2 )
    {
        pstRect->iX1 = iX1;
        pstRect->iX2 = iX2;
    }
    else
    {
        pstRect->iX1 = iX2;
        pstRect->iX2 = iX1;
    }
    
    // y1 < y2�� �ǵ��� RECT �ڷᱸ���� Y��ǥ�� ����
    if( iY1 < iY2 )
    {
        pstRect->iY1 = iY1;
        pstRect->iY2 = iY2;
    }
    else
    {
        pstRect->iY1 = iY2;
        pstRect->iY2 = iY1;
    }
}

/**
 *  �� �׸���
 */
inline void InternalDrawPixel( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, 
        int iX, int iY, COLOR stColor )
{
    int iWidth;
    
    // Ŭ���� ó��
    // ȭ�鿡 ǥ�õǴ� ������ ����ٸ� �׸��� ����
    if( IsInRectangle( pstMemoryArea, iX, iY ) == FALSE )
    {
        return ;
    }
    
    // ����� �޸� ������ �ʺ� ����
    iWidth = GetRectangleWidth( pstMemoryArea );
    
    // �ȼ� ���������� ����Ͽ� �ȼ� ���
    *( pstMemoryAddress + ( iWidth * iY ) + iX ) = stColor;
}

/**
 *  ���� �׸���
 */
void InternalDrawLine( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, 
        int iX1, int iY1, int iX2, int iY2, COLOR stColor )
{
    int iDeltaX, iDeltaY;
    int iError = 0;
    int iDeltaError;
    int iX, iY;
    int iStepX, iStepY;
    RECT stLineArea;
    
    // Ŭ���� ó��
    // ������ �׷����� ������ �޸� ������ ��ġ�� ������ �׸��� �ʾƵ� ��
    SetRectangleData( iX1, iY1, iX2, iY2, &stLineArea );
    if( IsRectangleOverlapped( pstMemoryArea, &stLineArea ) == FALSE )
    {
        return ;
    }
    
    // ��ȭ�� ���
    iDeltaX = iX2 - iX1;
    iDeltaY = iY2 - iY1;

    // X�� ��ȭ���� ���� X�� ���� ���� ���
    if( iDeltaX < 0 ) 
    {
        iDeltaX = -iDeltaX; 
        iStepX = -1; 
    } 
    else 
    { 
        iStepX = 1; 
    }

    // Y�� ��ȭ���� ���� Y�� ���� ���� ��� 
    if( iDeltaY < 0 ) 
    {
        iDeltaY = -iDeltaY; 
        iStepY = -1; 
    } 
    else 
    {
        iStepY = 1; 
    }

    // X�� ��ȭ���� Y�� ��ȭ������ ũ�ٸ� X���� �߽����� ������ �׸�
    if( iDeltaX > iDeltaY )
    {
        // ����� �� �ȼ����� ������ ����, Y�� ��ȭ���� 2��
        // ����Ʈ �������� * 2�� ��ü
        iDeltaError = iDeltaY << 1;
        iY = iY1;
        for( iX = iX1 ; iX != iX2 ; iX += iStepX )
        {
            // �� �׸���
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, iX, iY, stColor );

            // ���� ����
            iError += iDeltaError;

            // ������ ������ X�� ��ȭ������ ũ�� ���� ���� �����ϰ� ������ ���� ����
            // �������� ����
            if( iError >= iDeltaX )
            {
                iY += iStepY;
                // X���� ��ȭ���� 2�踦 ����
                // ����Ʈ �������� *2�� ��ü
                iError -= iDeltaX << 1;
            }
        }
        
        // iX == iX2�� ���� ��ġ�� �� �׸���
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, iX, iY, stColor );
    }
    // Y�� ��ȭ���� X�� ��ȭ������ ũ�ų� ���ٸ� Y���� �߽����� ������ �׸�
    else
    {
        // ����� �� �ȼ����� ������ ����, X�� ��ȭ���� 2��
        // ����Ʈ �������� * 2�� ��ü
        iDeltaError = iDeltaX << 1;
        iX = iX1;
        for( iY = iY1 ; iY != iY2 ; iY += iStepY )
        {
            // �� �׸���
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, iX, iY, stColor );

            // ���� ����
            iError += iDeltaError;

            // ������ ������ Y�� ��ȭ������ ũ�� ���� ���� �����ϰ� ������ ���� ����
            // �������� ����
            if( iError >= iDeltaY )
            {
                iX += iStepX;
                // Y���� ��ȭ���� 2�踦 ����
                // ����Ʈ �������� *2�� ��ü
                iError -= iDeltaY << 1;
            }
        }

        // iY == iY2�� ���� ��ġ�� �� �׸���
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, iX, iY, stColor );
    }
}

/**
 *  �簢�� �׸���
 */
void InternalDrawRect( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill )
{
    int iWidth;
    int iTemp;
    int iY;
    int iMemoryAreaWidth;
    RECT stDrawRect;
    RECT stOverlappedArea;

    // ä�� ���ο� ���� �ڵ带 �и�
    if( bFill == FALSE )
    {
        // �� ���� �̿��� �ͳ��� �������� ����
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, iX1, iY1, iX2, iY1, stColor );
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, iX1, iY1, iX1, iY2, stColor );
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, iX2, iY1, iX2, iY2, stColor );
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, iX1, iY2, iX2, iY2, stColor );
    }
    else
    {
        // ����� �簢���� ������ RECT �ڷᱸ���� ����
        SetRectangleData( iX1, iY1, iX2, iY2, &stDrawRect );
        
        // ����� �޸� ������ �簢�� ������ ��ġ�� �κ��� ����Ͽ� Ŭ���� ó��
        if( GetOverlappedRectangle( pstMemoryArea, &stDrawRect, 
                &stOverlappedArea ) == FALSE )
        {
            // ��ġ�� ������ ������ �׸� �ʿ� ����
            return ;
        }
                
        // Ŭ���ε� �簢���� �ʺ� ���
        iWidth = GetRectangleWidth( &stOverlappedArea );
        
        // ����� �޸� ������ �ʺ� ���
        iMemoryAreaWidth = GetRectangleWidth( pstMemoryArea );
        
        // ����� �޸� ��巹���� ���� ��ġ�� ���
        // �Ķ���ͷ� ���޵� �簢���� �״�� �׸��� ���� �ƴ϶� Ŭ���� ó���� �簢����
        // �������� �׸�
        pstMemoryAddress += stOverlappedArea.iY1 * iMemoryAreaWidth + 
            stOverlappedArea.iX1;
        
        // ������ ���鼭 �� Y�ึ�� ���� ä��
        for( iY = stOverlappedArea.iY1 ; iY < stOverlappedArea.iY2 ; iY++ )
        {
            // �޸𸮿� �簢���� �ʺ�ŭ �ȼ��� ä��
            MemSetWord( pstMemoryAddress, stColor, iWidth );
            
            // ����� ���� �޸� ��巹�� ����
            // x, y��ǥ�� �Ź� ���� �޸� ��巹���� ����ϴ� ���� ���Ϸ���
            // X�� �ػ󵵸� �̿��Ͽ� ���� ������ y��ǥ ��巹���� ��� 
            pstMemoryAddress += iMemoryAreaWidth;
        }
        
        // �޸𸮿� �簢���� �ʺ�ŭ �ȼ��� ä��, ������ �� ���
        MemSetWord( pstMemoryAddress, stColor, iWidth );
    }
}

/**
 *  �� �׸���
 */
void InternalDrawCircle( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, int iRadius, COLOR stColor, BOOL bFill )
{
    int iCircleX, iCircleY;
    int iDistance;
    
    // �������� 0���� �۴ٸ� �׸� �ʿ� ����
    if( iRadius < 0 )
    {
        return ;
    }
    
    // (0, R)�� ��ǥ���� ����
    iCircleY = iRadius;

    // ä�� ���ο� ���� �������� �׸�
    if( bFill == FALSE )
    {
        // �������� �� ���� ��� �׸�
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 0 + iX, iRadius + iY, stColor);
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 0 + iX, -iRadius + iY, stColor);
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, iRadius + iX, 0 + iY, stColor);
        InternalDrawPixel( pstMemoryArea, pstMemoryAddress, -iRadius + iX, 0 + iY, stColor);
    }
    else
    {
        // ���� ������ X��� Y�� ��� �׸�
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, 
                   0 + iX, iRadius + iY, 0 + iX, -iRadius + iY, stColor);
        InternalDrawLine( pstMemoryArea, pstMemoryAddress, 
                iRadius + iX, 0 + iY, -iRadius + iX, 0 + iY, stColor);
    }
    
    // ���� �������� �߽����� ���� �Ÿ�
    iDistance = -iRadius;

    // �� �׸���
    for( iCircleX = 1 ; iCircleX <= iCircleY ; iCircleX++ )
    {
        // ������ ������ �Ÿ� ���
        // ����Ʈ �������� * 2�� ��ü
        iDistance += ( iCircleX << 1 ) - 1;  //2 * iCircleX - 1;
                    
        // �߽����� ���� �ܺο� ������ �Ʒ��� �ִ� �� ����
        if( iDistance >= 0 )
        {
            iCircleY--;
            
            // ���ο� ������ �ٽ� ���� �Ÿ� ���
            // ����Ʈ �������� * 2�� ��ü
            iDistance += ( -iCircleY << 1 ) + 2; //-2 * iCircleY + 2;
        }
        
        // ä�� ���ο� ���� �׸�
        if( bFill == FALSE )
        {
            // 8 ���� ��� �� �׸�
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    iCircleX + iX, iCircleY + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    iCircleX + iX, -iCircleY + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    -iCircleX + iX, iCircleY + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    -iCircleX + iX, -iCircleY + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    iCircleY + iX, iCircleX + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    iCircleY + iX, -iCircleX + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    -iCircleY + iX, iCircleX + iY, stColor );
            InternalDrawPixel( pstMemoryArea, pstMemoryAddress, 
                    -iCircleY + iX, -iCircleX + iY, stColor );
        }
        else
        {
            // ��Ī�Ǵ� ���� ã�� X�࿡ ������ ������ �׾� ä���� ���� �׸�
            // ���༱�� �׸��� ���� �簢�� �׸��� �Լ��� ������ ó���� �� ����
            InternalDrawRect( pstMemoryArea, pstMemoryAddress, -iCircleX + iX, iCircleY + iY, 
                    iCircleX + iX, iCircleY + iY, stColor, TRUE );
            InternalDrawRect( pstMemoryArea, pstMemoryAddress, -iCircleX + iX, -iCircleY + iY, 
                    iCircleX + iX, -iCircleY + iY, stColor, TRUE );
            InternalDrawRect( pstMemoryArea, pstMemoryAddress, -iCircleY + iX, iCircleX + iY, 
                    iCircleY + iX, iCircleX + iY, stColor, TRUE );
            InternalDrawRect( pstMemoryArea, pstMemoryAddress, -iCircleY + iX, -iCircleX + iY, 
                    iCircleY + iX, -iCircleX + iY, stColor, TRUE );
        }
    }
}

/**
 *  ���� ���
 */
void InternalDrawText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor,
        const char* pcString, int iLength )
{
    int i;
    int j;

    for( i = 0 ; i < iLength ; )
    {
        // ���� ���ڰ� �ѱ��� �ƴϸ� �����ڰ� ������ ���� �˻�
        if( ( pcString[ i ] & 0x80 ) == 0 )
        {
            // ���ڿ��� ������ �˻�
            for( j = i ; j < iLength ; j++ )
            {
                if( pcString[ j ] & 0x80 )
                {
                    break;
                }
            }

            // �����ڸ� ����ϴ� �Լ��� ȣ���ϰ� ���� ��ġ�� ����
            InternalDrawEnglishText( pstMemoryArea, pstMemoryAddress,
                    iX + ( i * FONT_ENGLISHWIDTH ), iY, stTextColor, stBackgroundColor,
                    pcString + i, j - i );
            i = j;
        }
        // ���� ���ڰ� �ѱ��̸� �ѱ��� ������ ���� �˻�
        else
        {
            // ���ڿ��� ������ �˻�
            for( j = i ; j < iLength ; j++ )
            {
                if( ( pcString[ j ] & 0x80 ) == 0 )
                {
                    break;
                }
            }

            // �����ڸ� ����ϴ� �Լ��� ȣ���ϰ� ���� ��ġ�� ����
            InternalDrawHangulText( pstMemoryArea, pstMemoryAddress,
                    iX + i * FONT_ENGLISHWIDTH, iY, stTextColor, stBackgroundColor,
                    pcString + i, j - i );
            i = j;
        }
    }
}

/**
 *  ������ ���� ���
 */
void InternalDrawEnglishText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, 
        const char* pcString, int iLength )
{
    int iCurrentX, iCurrentY;
    int i, j, k;
    BYTE bBitmap;
    BYTE bCurrentBitmask;
    int iBitmapStartIndex;
    int iMemoryAreaWidth;
    RECT stFontArea;
    RECT stOverlappedArea;
    int iStartYOffset;
    int iStartXOffset;
    int iOverlappedWidth;
    int iOverlappedHeight;

    // ���ڸ� ����ϴ� X��ǥ
    iCurrentX = iX;
    
    // �޸� ������ �ʺ� ���
    iMemoryAreaWidth = GetRectangleWidth( pstMemoryArea );
    
    // ������ ������ŭ �ݺ�
    for( k = 0 ; k < iLength ; k++ )
    {
        // ���ڸ� ����� ��ġ�� Y��ǥ�� ����
        iCurrentY = iY * iMemoryAreaWidth;

        // ���� ��Ʈ�� ǥ���ϴ� ������ RECT �ڷᱸ���� ����
        SetRectangleData( iCurrentX, iY, iCurrentX + FONT_ENGLISHWIDTH - 1,
                iY + FONT_ENGLISHHEIGHT - 1, &stFontArea );

        // ���� �׷��� �� ���ڰ� �޸� ������ ��ġ�� �κ��� ������ ���� ���ڷ� �̵�
        if( GetOverlappedRectangle( pstMemoryArea, &stFontArea, 
                                     &stOverlappedArea ) == FALSE )
        {
            // ���� �ϳ��� �پ�Ѿ����Ƿ� ��Ʈ�� �ʺ�ŭ x��ǥ�� �̵��Ͽ� ���� ���ڸ� ���
            iCurrentX += FONT_ENGLISHWIDTH;
            continue;
        }
        
        // ��Ʈ�� ��Ʈ �����Ϳ��� ����� ������ ��Ʈ���� �����ϴ� ��ġ�� ���
        // 1����Ʈ * FONT_HEIGHT�� �����Ǿ� �����Ƿ� ������ ��Ʈ�� ��ġ��
        // �Ʒ��� ���� ��� ����
        iBitmapStartIndex = pcString[ k ] * FONT_ENGLISHHEIGHT;

        // ���ڸ� ����� ������ �޸� ������ ��ġ�� �κ��� �̿��Ͽ� x, y�����°�
        // ����� �ʺ�, ���̸� ���
        iStartXOffset = stOverlappedArea.iX1 - iCurrentX;
        iStartYOffset = stOverlappedArea.iY1 - iY;
        iOverlappedWidth = GetRectangleWidth( &stOverlappedArea );
        iOverlappedHeight = GetRectangleHeight( &stOverlappedArea );

        // ��¿��� ���ܵ� y�����¸�ŭ ��Ʈ�� �����͸� ����
        iBitmapStartIndex += iStartYOffset;        
        
        // ���� ���
        // ��ġ�� ������ y�����º��� ���̸�ŭ ���
        for( j = iStartYOffset ; j < iOverlappedHeight ; j++ )
        {
            // �̹� ���ο��� ����� ��Ʈ ��Ʈ�ʰ� ��Ʈ ������ ���
            bBitmap = g_vucEnglishFont[ iBitmapStartIndex++ ];
            bCurrentBitmask = 0x01 << ( FONT_ENGLISHWIDTH - 1 - iStartXOffset );
            
            // ��ġ�� ������ x�����º��� �ʺ�ŭ ���� ���ο� ���
            for( i = iStartXOffset ; i < iOverlappedWidth ; i++ )
            {
                // ��Ʈ�� �����Ǿ������� ȭ�鿡 ���ڻ��� ǥ��
                if( bBitmap & bCurrentBitmask )
                {
                    pstMemoryAddress[ iCurrentY + iCurrentX + i ] = stTextColor;
                }
                // ��Ʈ�� �����Ǿ����� ������ ȭ�鿡 ������ ǥ��
                else
                {
                    pstMemoryAddress[ iCurrentY + iCurrentX + i ] = stBackgroundColor;
                }
                
                bCurrentBitmask = bCurrentBitmask >> 1;
            }
            
            // ���� �������� �̵��ؾ� �ϹǷ�, ���� y��ǥ�� �޸� ������ �ʺ�ŭ ������
            iCurrentY += iMemoryAreaWidth;
        }
        
        // ���� �ϳ��� �� ��������� ��Ʈ�� ���̸�ŭ X ��ǥ�� �̵��Ͽ� ���� ���ڸ� ���
        iCurrentX += FONT_ENGLISHWIDTH;
    }
}

/**
 *  �ѱ� ���
 */
void InternalDrawHangulText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor,
        const char* pcString, int iLength )
{
    int iCurrentX, iCurrentY;
    WORD wHangul;
    WORD wOffsetInGroup;
    WORD wGroupIndex;
    int i, j, k;
    WORD wBitmap;
    WORD wCurrentBitmask;
    int iBitmapStartIndex;
    int iMemoryAreaWidth;
    RECT stFontArea;
    RECT stOverlappedArea;
    int iStartYOffset;
    int iStartXOffset;
    int iOverlappedWidth;
    int iOverlappedHeight;

    // ���ڸ� ����ϴ� X��ǥ
    iCurrentX = iX;

    // �޸� ������ �ʺ� ���
    iMemoryAreaWidth = GetRectangleWidth( pstMemoryArea );

    // �ѱ� ������ ������ŭ �ݺ�
    for( k = 0 ; k < iLength ; k += 2 )
    {
        // ���ڸ� ����� ��ġ�� Y��ǥ�� ����
        iCurrentY = iY * iMemoryAreaWidth;

        // ���� ��Ʈ�� ǥ���ϴ� ������ RECT �ڷᱸ���� ����
        SetRectangleData( iCurrentX, iY, iCurrentX + FONT_HANGULWIDTH - 1,
                iY + FONT_HANGULHEIGHT - 1, &stFontArea );

        // ���� �׷��� �� ���ڰ� �޸� ������ ��ġ�� �κ��� ������ ���� ���ڷ� �̵�
        if( GetOverlappedRectangle( pstMemoryArea, &stFontArea,
                                     &stOverlappedArea ) == FALSE )
        {
            // ���� �ϳ��� �پ�Ѿ����Ƿ� ��Ʈ�� �ʺ�ŭ x��ǥ�� �̵��Ͽ� ���� ���ڸ� ���
            iCurrentX += FONT_HANGULWIDTH;
            continue;
        }

        // ��Ʈ�� ��Ʈ �����Ϳ��� ����� ������ ��Ʈ���� �����ϴ� ��ġ�� ���
        // 2����Ʈ * FONT_HEIGHT�� �����Ǿ� �����Ƿ� ������ ��Ʈ�� ��ġ��
        // �Ʒ��� ���� ��� ����
        // ����Ʈ�� ����� ��ȯ
        wHangul = ( ( WORD ) pcString[ k ] << 8 ) | ( BYTE ) ( pcString[ k + 1 ] );

        // �ϼ��� ��~�������̸� ��/�� �������� ������
        if( ( 0xB0A1 <= wHangul ) && ( wHangul <= 0xC8FE ) )
        {
            wOffsetInGroup = ( wHangul - 0xB0A1 ) & 0xFF;
            wGroupIndex = ( ( wHangul - 0xB0A1 ) >> 8 ) & 0xFF;
            // �׷�� 94�� ���ڰ� �ְ� 51���� �ϼ����� ���� �ڸ� ��������Ƿ� �׷� �ε����� 94�� ���� ��
            // �׷� �� �����¿� 51�� ���ϸ� ��Ʈ �����Ϳ��� �� ��°���� ����� �� ����
            wHangul = ( wGroupIndex * 94 ) + wOffsetInGroup + 51;
        }
        // ���� ��/���̸� ������ ������ ���� ���� �������� ����
        else if( ( 0xA4A1 <= wHangul ) && ( wHangul <= 0xA4D3 ) )
        {
            wHangul = wHangul - 0xA4A1;
        }
        // ���� �� ���� ��찡 �ƴϸ� ó���� �� �����Ƿ� ���� ���ڷ� �Ѿ
        else
        {
            continue ;
        }

        iBitmapStartIndex = wHangul * FONT_HANGULHEIGHT;

        // ���ڸ� ����� ������ �޸� ������ ��ġ�� �κ��� �̿��Ͽ� x, y�����°�
        // ����� �ʺ�, ���̸� ���
        iStartXOffset = stOverlappedArea.iX1 - iCurrentX;
        iStartYOffset = stOverlappedArea.iY1 - iY;
        iOverlappedWidth = GetRectangleWidth( &stOverlappedArea );
        iOverlappedHeight = GetRectangleHeight( &stOverlappedArea );

        // ��¿��� ���ܵ� y�����¸�ŭ ��Ʈ�� �����͸� ����
        iBitmapStartIndex += iStartYOffset;

        // ���� ���
        // ��ġ�� ������ y�����º��� ���̸�ŭ ���
        for( j = iStartYOffset ; j < iOverlappedHeight ; j++ )
        {
            // �̹� ���ο��� ����� ��Ʈ ��Ʈ�ʰ� ��Ʈ ������ ���
            wBitmap = g_vusHangulFont[ iBitmapStartIndex++ ];
            wCurrentBitmask = 0x01 << ( FONT_HANGULWIDTH - 1 - iStartXOffset );

            // ��ġ�� ������ x�����º��� �ʺ�ŭ ���� ���ο� ���
            for( i = iStartXOffset ; i < iOverlappedWidth ; i++ )
            {
                // ��Ʈ�� �����Ǿ������� ȭ�鿡 ���ڻ��� ǥ��
                if( wBitmap & wCurrentBitmask )
                {
                    pstMemoryAddress[ iCurrentY + iCurrentX + i ] = stTextColor;
                }
                // ��Ʈ�� �����Ǿ����� ������ ȭ�鿡 ������ ǥ��
                else
                {
                    pstMemoryAddress[ iCurrentY + iCurrentX + i ] = stBackgroundColor;
                }

                wCurrentBitmask = wCurrentBitmask >> 1;
            }

            // ���� �������� �̵��ؾ� �ϹǷ�, ���� y��ǥ�� �޸� ������ �ʺ�ŭ ������
            iCurrentY += iMemoryAreaWidth;
        }

        // ���� �ϳ��� �� ��������� ��Ʈ�� ���̸�ŭ X ��ǥ�� �̵��Ͽ� ���� ���ڸ� ���
        iCurrentX += FONT_HANGULWIDTH;
    }
}