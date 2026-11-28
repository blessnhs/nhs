#pragma once

/// @brief 응답 데이터를 호출자에게 전달하는 콜백 인터페이스입니다.
class IResponse
{
public:
	/// @brief 응답 인터페이스를 생성합니다.
	IResponse(void){}
	/// @brief 응답 인터페이스의 가상 소멸자입니다.
	virtual ~IResponse(void){}

	/// @brief 파생 클래스에서 응답 데이터를 처리합니다.
	virtual VOID OnResponse(LPVOID Data) = 0;
};

