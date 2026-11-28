#include "StdAfx.h"
#include "GSArgument.h"
#include <cstdlib>


GSArgument::GSArgument(void)
{
	m_MaxClient = 1000;
	m_BindPort  = 8245;
	m_LogicThreadCnt = 8;
	m_Version = 0;
	m_Naggle = false;
	m_AliveTime = 10000;
}


GSArgument::~GSArgument(void)
{
}

namespace
{
	bool ReadBindNumber(const char* iniPath, const char* key, const char* defaultValue,
		unsigned long limit, unsigned long& result)
	{
		char text[32]{};
		const auto length = GetPrivateProfileStringA("SERVER", key, defaultValue, text,
			static_cast<DWORD>(sizeof(text)), iniPath);
		if (!length || length >= sizeof(text) - 1 ||
			std::string(text).find_first_not_of("0123456789") != std::string::npos)
			return false;
		result = std::strtoul(text, nullptr, 10);
		return result <= limit;
	}
}

bool GSArgument::LoadTcpBindEndpoints(const char* iniPath)
{
	unsigned long count = 0;
	if (!ReadBindNumber(iniPath, "TCP_BIND_COUNT", "0", MAXWORD, count) ||
		count > m_MaxClient)
		return false;

	std::vector<TcpBindEndpoint> endpoints;
	endpoints.reserve(count);
	for (unsigned long i = 1; i <= count; ++i)
	{
		const auto suffix = std::to_string(i);
		const auto portKey = "TCP_BIND_PORT_" + suffix;
		unsigned long port = 0;
		if (!ReadBindNumber(iniPath, portKey.c_str(), "", MAXWORD, port) || !port)
			return false;

		const auto ipKey = "TCP_BIND_IP_" + suffix;
		char address[32]{};
		const auto length = GetPrivateProfileStringA("SERVER", ipKey.c_str(), "0.0.0.0", address,
			static_cast<DWORD>(sizeof(address)), iniPath);
		IN_ADDR ipv4{};
		if (!length || length >= sizeof(address) - 1 || InetPtonA(AF_INET, address, &ipv4) != 1)
			return false;

		endpoints.push_back({address, static_cast<WORD>(port)});
	}
	m_TcpBindEndpoints.swap(endpoints);
	return true;
}
