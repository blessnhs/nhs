// Front.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include "./Command/Command.h"
#include "Constructor/Constructor.h"

int main()
{
	try
	{
		FrontCommand command;
		FrontConstructor server;
		return EXIT_SUCCESS;
	}
	catch (const std::exception& error)
	{
		SYSLOG().Write("[Front/main] Fatal exception detail=%.512s\n", error.what());
	}
	catch (...)
	{
		SYSLOG().Write("[Front/main] Fatal unknown exception\n");
	}

	fprintf(stderr, "Front stopped after a fatal error. See the system log.\n");
	return EXIT_FAILURE;
}
