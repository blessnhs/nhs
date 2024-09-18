/**
 *  file    2DGraphics.h
 *  date    2009/09/5
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   2D Graphic�� ���� ��� ����
 */

#ifndef __2DGRAPHICS_H__
#define __2DGRAPHICS_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �����ϴµ� ����� �ڷᱸ��, 16��Ʈ ���� ����ϹǷ� WORD�� ����
typedef WORD                COLOR;

// 0~255 ������ R, G, B�� 16��Ʈ �� �������� ��ȯ�ϴ� ��ũ��
// 0~255�� ������ 0~31, 0~63���� ����Ͽ� ����ϹǷ� ���� 8�� 4�� ��������� ��
// ������ 8�� ������ 4�� >> 3�� >> 2�� ��ü
#define RGB( r, g, b )      ( ( ( BYTE )( r ) >> 3 ) << 11 | \
                ( ( ( BYTE )( g ) >> 2 ) ) << 5 |  ( ( BYTE )( b ) >> 3 ) )


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// �簢���� ������ ��� �ڷᱸ��
typedef struct kRectangleStruct
{
    // ���� ��(������)�� X ��ǥ
    int iX1;
    // ���� ��(������)�� Y ��ǥ
    int iY1;
    
    // ������ �Ʒ�(����)�� X ��ǥ
    int iX2;
    // ������ �Ʒ�(����)�� Y��ǥ
    int iY2;
} RECT;

// ���� ������ ��� �ڷᱸ��
typedef struct kPointStruct
{
    // X�� Y�� ��ǥ
    int iX;
    int iY;
} POINT;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
inline void InternalDrawPixel( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, 
        int iX, int iY, COLOR stColor );
void InternalDrawLine( const RECT* pstMemoryArea, COLOR* pstMemoryAddress, 
        int iX1, int iY1, int iX2, int iY2, COLOR stColor );
void InternalDrawRect( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill );
void InternalDrawCircle( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, int iRadius, COLOR stColor, BOOL bFill );
void InternalDrawText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, 
        const char* pcString, int iLength );
void InternalDrawEnglishText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor,
        const char* pcString, int iLength );
void InternalDrawHangulText( const RECT* pstMemoryArea, COLOR* pstMemoryAddress,
        int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor,
        const char* pcString, int iLength );

inline BOOL IsInRectangle( const RECT* pstArea, int iX, int iY );
inline int GetRectangleWidth( const RECT* pstArea );
inline int GetRectangleHeight( const RECT* pstArea );
inline void SetRectangleData( int iX1, int iY1, int iX2, int iY2, RECT* pstRect );
inline BOOL GetOverlappedRectangle( const RECT* pstArea1, const RECT* pstArea2, 
        RECT* pstIntersection  );
inline BOOL IsRectangleOverlapped( const RECT* pstArea1, 
        const RECT* pstArea2 );

#endif /*__2DGRAPHICS_H__*/
