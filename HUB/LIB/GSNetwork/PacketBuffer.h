#pragma once



/// @brief 수신 바이트를 누적하고 선두 위치로 소비량을 관리하는 가변 크기 버퍼입니다.
class GSPacketBuffer
{
public:
	/// @brief 지정 크기의 저장소를 할당하고 헤드와 테일을 초기화합니다.
	GSPacketBuffer(DWORD dwBufferSize=16384);
	/// @brief 저장소를 해제합니다.
	~GSPacketBuffer();
	 
	 /// @brief 입력 바이트를 버퍼 끝에 추가합니다.
	 bool PutData(char* pData, DWORD length);	// Input to BufferQueue Received Data from Socket
	 /// @brief 선두부터 지정한 길이의 데이터를 호출자 버퍼로 복사합니다.
	 bool GetData(char* pData, DWORD length);	// Output Data from BufferQueue. length is requesting data size
	 /// @brief 현재 남아 있는 데이터의 길이를 반환합니다.
	 DWORD GetValidDataLength();				// Get Length of Valid Data from BufferQueue 
	 /// @brief 지정한 길이만큼 읽기 위치를 전진시킵니다.
	 void MoveHeadPos(DWORD length);			// Data Current Head Pointer move an extent of length (for Data Delete)
	 /// @brief 남은 데이터를 보존하면서 버퍼 용량을 늘립니다.
	 void ReSize();							// Resize BufferQueue for BufferQueue overflow
	 /// @brief 헤드와 테일을 초기 위치로 되돌립니다.
	 void SetInit() { m_dwHeadPos=0; m_dwTailPos=0;};
	 /// @brief 지정 길이를 추가할 때 현재 버퍼 공간을 초과하는지 확인합니다.
	 bool IsOverflow(DWORD length);			// Estimation of Buffersize overflow
	 /// @brief 현재 읽기 위치를 반환합니다.
	 DWORD GetHeadPos() { return m_dwHeadPos;};
	 /// @brief 현재 쓰기 위치를 반환합니다.
	 DWORD GetTailPos() { return m_dwTailPos;};

private:
	char* m_pBuffer;
	DWORD m_dwBufferSize;
	DWORD m_dwHeadPos;
	DWORD m_dwTailPos;
};

