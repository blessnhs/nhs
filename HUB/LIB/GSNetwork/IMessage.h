#pragma once

#include "IHandler.h"
#include <boost/shared_ptr.hpp>

namespace GSNetwork	{
		
/// @brief 작업 스레드가 실행할 메시지와 처리에 필요한 정보를 담는 기본 클래스입니다.
class IMessage : public GSNetwork::GSObject::GSObject	
{
public:
	/// @brief 메시지와 처리기 참조를 빈 상태로 초기화합니다.
	IMessage() { Message = NULL; pHandler = NULL; }
	/// @brief 메시지의 가상 소멸자입니다.
	virtual ~IMessage(){}

	IHandler<boost::shared_ptr<IMessage>>	*pHandler;
	LPVOID		Message;
	int			Type;
	int			SubType;

	/// @brief 파생 메시지가 구현하는 작업 실행 훅입니다.
	virtual void Execute(LPVOID Param){};

};


}

#define IMESSAGE GSNetwork::IMessage

typedef boost::shared_ptr<GSNetwork::IMessage> IMessagePtr;
