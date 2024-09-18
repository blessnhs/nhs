#include "StdAfx.h"
#include "Constructor.h"

#include "../Command/Command.h"
#include "../Server/GSFront.h"
#include "CrashHandler/SFCrashHandler.h"
#include "../Timer/ContentsTimer.h"

FrontConstructor::FrontConstructor(void)
{
	//������Ʈ �Ӽ��� c/c++ => �ڵ� ���� �׸񿡼� �⺻ ��Ÿ�� �˻縦 �⺻������ �����ؾ���
	SFCrashHandler handler;
	handler.Install();
	handler.SetFHPtr(nullptr);

	SERVER.Initialize();
	PROXYHUB.Create(INI.HubSessionCount);

	GetContentsTimer().Start();
	
	printf("Server Start.................. \n");

	FrontCommand *_Command = new FrontCommand();
	
	while(TRUE)
	{
		CHAR command[256];
		memset(command, 0, sizeof(command));
		scanf_s("%s",command,(int)sizeof(command));

		_Command->Execute(command);
	}

	delete _Command;
}


FrontConstructor::~FrontConstructor(void)
{
}
