#pragma once

namespace GSNetwork	{	namespace GSMemPool	{	namespace GSBUffer	{

/// @brief 소유 바이트 배열과 읽기 위치를 관리하는 바이너리 직렬화 버퍼입니다. 호출자가 읽기·쓰기 범위를 보장해야 합니다.
class GSBuffer // : public GSMemPool<GSBuffer>
{
public:
	/// @brief 빈 버퍼와 초기 읽기 위치를 설정합니다.
	GSBuffer(void);
	/// @brief 버퍼가 소유한 배열을 해제합니다.
	~GSBuffer(void);

private:
	BYTE	*mBufferPointer;
	DWORD	mLength;
	DWORD   mPos;

public:

	/// @brief 지정 크기의 소유 버퍼를 할당하고 시작 주소를 반환합니다.
	BYTE*   AllocBuffer(INT Size);
	/// @brief 입력 배열이 있으면 데이터를 복사하여 저장하며, 인수 없는 오버로드는 현재 길이를 0으로 설정합니다.
	BOOL	SetBuffer(BYTE *buffer,INT Size);

	/// @brief 입력 배열이 있으면 데이터를 복사하여 저장하며, 인수 없는 오버로드는 현재 길이를 0으로 설정합니다.
	BOOL	SetBuffer();
	/// @brief 소유 버퍼의 시작 주소를 반환합니다.
	BYTE	*GetBuffer();

	/// @brief 현재 위치에서 Int32 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadInt32(INT *data);
	/// @brief 현재 위치에서 DWORD 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadDWORD(DWORD *data);
	/// @brief 현재 위치에서 DWORD_PTR 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadDWORD_PTR(DWORD_PTR *data);
	/// @brief 현재 위치에서 Byte 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadByte(BYTE *data);
	/// @brief 현재 위치에서 Bytes 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadBytes(BYTE *data, DWORD length);
	/// @brief 현재 위치에서 Float 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadFloat(FLOAT *data);
	/// @brief 현재 위치에서 Int64 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadInt64(INT64 *data);
	/// @brief 현재 위치에서 USHORT 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadUSHORT(USHORT *data);
	/// @brief 현재 위치에서 SHORT 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadSHORT(SHORT *data);
	/// @brief 현재 위치에서 BOOL 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadBOOL(BOOL *data);
	/// @brief 현재 위치에서 WCHAR 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadWCHAR(WCHAR *data);
	/// @brief 현재 위치에서 WCHARs 데이터를 읽고 읽기 위치를 전진시킵니다.
	BOOL	ReadWCHARs(LPWSTR data, DWORD length);

	/// @brief Int32 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteInt32(INT data);
	/// @brief DWORD 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteDWORD(DWORD data);
	/// @brief DWORD_PTR 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteDWORD_PTR(DWORD_PTR data);
	/// @brief Byte 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteByte(BYTE data);
	/// @brief Bytes 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteBytes(BYTE *data, DWORD length);
	/// @brief Float 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteFloat(FLOAT data);
	/// @brief Int64 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteInt64(INT64 data);
	/// @brief USHORT 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteUSHORT(USHORT data);
	/// @brief SHORT 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteSHORT(SHORT data);
	/// @brief BOOL 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteBOOL(BOOL data);
	/// @brief WCHAR 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteWCHAR(WCHAR data);
	/// @brief WCHARs 데이터를 버퍼 끝에 기록하고 길이를 갱신합니다.
	BOOL	WriteWCHARs(LPCWSTR data, DWORD length);

	/// @brief 현재 버퍼의 논리적 데이터 길이를 반환합니다.
	DWORD	GetLength(VOID);
};

}	}	}

#define XBUFFER GSNetwork::GSMemPool::GSBUffer::GSBuffer