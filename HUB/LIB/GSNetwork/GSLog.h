#pragma once
#include <stdio.h>
#include <windows.h>
#include <iostream>

#define MAX_LOG_LEN  2048

using namespace std;

/// @brief 서버 진단 메시지를 파일에 기록하고 설정에 따라 콘솔에도 표시하는 로그입니다.
class GSLog
{
private:
	bool _display;

	CRITICAL_SECTION cs;
	HANDLE			handle_;
	string			_name;
	DWORD			_openDate;		// [LOCKFIX] 현재 열린 파일의 날짜 (yyyymmdd). 날짜가 바뀔 때만 다시 연다.

public:

	/// @brief 로그 이름과 콘솔 표시 여부를 설정합니다.
	GSLog(string name,bool display = false);


	/// @brief 현재 로그 파일을 엽니다.
	void Open();

	/// @brief 열린 로그 파일을 닫습니다.
	void Close();

	/// @brief 로그 객체가 사용한 리소스를 정리합니다.
	~GSLog(void);


	/// @brief 서식 문자열과 인수를 조합해 로그 메시지를 기록합니다.
	bool			Write(const char* fmt, ...);
};

/// @brief 서버에서 공유하는 시스템 로그 객체를 반환합니다.
extern GSLog &SYSLOG();



