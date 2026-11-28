#include "stdafx.h"

#include "./Command/Command.h"
#include "Constructor/Constructor.h"

int main()
{
	try
	{
		HubCommand command;
		HubConstructor server;
		return EXIT_SUCCESS;
	}
	catch (const std::exception& error)
	{
		SYSLOG().Write("[HUB/main] Fatal exception detail=%.512s\n", error.what());
	}
	catch (...)
	{
		SYSLOG().Write("[HUB/main] Fatal unknown exception\n");
	}

	fprintf(stderr, "HUB stopped after a fatal error. See the system log.\n");
	return EXIT_FAILURE;
}
