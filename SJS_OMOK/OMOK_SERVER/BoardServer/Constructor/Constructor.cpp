#include "StdAfx.h"
#include "Constructor.h"

#include "../Command/Command.h"
#include "../Server/GSBoard.h"
#include "CrashHandler/SFCrashHandler.h"
#include "../Timer/ContentsTimer.h"

BoardConstructor::BoardConstructor(void)
{
	//������Ʈ �Ӽ��� c/c++ => �ڵ� ���� �׸񿡼� �⺻ ��Ÿ�� �˻縦 �⺻������ �����ؾ���
	SFCrashHandler handler;
	handler.Install();
	handler.SetFHPtr(nullptr);

	SERVER.Initialize();

	GetContentsTimer().Start();
	
	printf("Server Start.................. \n");

	BoardCommand *_Command = new BoardCommand();
	
	while(TRUE)
	{
		CHAR command[256];
		memset(command, 0, sizeof(command));
		scanf_s("%s",command,(int)sizeof(command));

		_Command->Execute(command);
	}

	delete _Command;
}


BoardConstructor::~BoardConstructor(void)
{
}
