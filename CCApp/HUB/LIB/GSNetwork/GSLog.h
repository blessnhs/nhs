#pragma once
#include <stdio.h>
#include <windows.h>
#include <iostream>

#define MAX_LOG_LEN  2048

using namespace std;

class GSLog
{
private:
	bool _display;

	CRITICAL_SECTION cs;
	HANDLE			handle_;
	string			_name;
	DWORD			_openDate;		// [LOCKFIX] 현재 열린 파일의 날짜 (yyyymmdd). 날짜가 바뀔 때만 다시 연다.

public:

	GSLog(string name,bool display = false);


	void Open();

	void Close();

	~GSLog(void);


	bool			Write(const char* fmt, ...);
};

extern GSLog &SYSLOG();



