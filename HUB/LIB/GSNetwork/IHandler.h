#pragma once

/// @brief 타입 T의 이벤트를 받아 처리하는 인터페이스입니다.
template<class T>
class IHandler : public GSNetwork::GSObject::GSObject
{
public:
	/// @brief 이벤트 처리 인터페이스를 생성합니다.
	IHandler() {}
	/// @brief 이벤트 처리 인터페이스의 가상 소멸자입니다.
	virtual  ~IHandler(){}

	/// @brief 이벤트를 받아 실제 구현의 처리 로직을 실행합니다.
	virtual void OnEvt(T Arg) = 0;
};