#pragma once

/// @brief 리스닝 포트, 작업 스레드 수, 접속 한도 등 네트워크 서버의 시작 설정입니다.
class GSArgument
{
public:
	/// @brief 서버 인수의 기본값을 설정합니다.
	GSArgument(void);
	/// @brief 서버 인수 객체를 파괴합니다.
	~GSArgument(void);

	//최대 접속 가능 클라이언트
	WORD m_MaxClient;	
	
	//tcp 바인드 포트
	WORD m_BindPort;

	/// @brief TCP 리스너 하나가 바인딩할 IPv4 주소와 포트입니다.
	struct TcpBindEndpoint
	{
		std::string Address;
		WORD Port;
	};

	/// @brief INI의 SERVER 섹션에서 번호가 붙은 TCP 바인딩 목록을 읽습니다.
	bool LoadTcpBindEndpoints(const char* iniPath);

	// An empty list uses the legacy wildcard address and m_BindPort.
	std::vector<TcpBindEndpoint> m_TcpBindEndpoints;

	//로직 쓰레드 카운트
	WORD m_LogicThreadCnt;

	//버전
	WORD m_Version;

	//나글 알고리즘
	BOOL m_Naggle;

	//얼라이브 타임 설정 second 
	DWORD m_AliveTime;

	//압축해제 사용하지 않음 릴레이 서버만 false
	BOOL m_UseCompress = true;

	//udp바인트 포트
	std::vector<DWORD> m_UdpPorts;

	//최대 db thread Count
	WORD m_DBThreadCnt;
};

