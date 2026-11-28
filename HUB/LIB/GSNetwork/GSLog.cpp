#include "StdAfx.h"
#include "GSLog.h"

// [LOCKFIX] 2026-11-26
//  이전: Write() 호출마다 락 안에서 CreateFile -> WriteFile -> CloseHandle + printf
//        (소켓 락 안에서도 로그를 쓰기 때문에 파일 I/O 시간만큼 다른 스레드가 대기)
//  변경: 파일 핸들을 열어 둔 채로 재사용하고 날짜가 바뀔 때만 새로 연다.
//        문자열 포맷과 printf는 락 밖에서 수행하고, 락은 WriteFile 구간에만 잡는다.
GSLog::GSLog(string name,bool display)
{
	_display = display;
	_name = name;
	handle_ = INVALID_HANDLE_VALUE;
	_openDate = 0;
	InitializeCriticalSection(&cs);
}

// cs를 잡은 상태에서 호출
void GSLog::Open()
{
	SYSTEMTIME		sysTime;
	::GetLocalTime(&sysTime);

	DWORD today = sysTime.wYear * 10000 + sysTime.wMonth * 100 + sysTime.wDay;

	if (handle_ != INVALID_HANDLE_VALUE && _openDate == today)
		return;

	Close();

	CHAR			time_log[256];

	sprintf_s(time_log, 256, "%d_%d_%d_%s.txt",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay, _name.c_str());

	// 열어 둔 채로 쓰므로 다른 프로그램이 읽을 수 있게 FILE_SHARE_READ 허용
	handle_ = CreateFile(time_log, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (handle_ == INVALID_HANDLE_VALUE) { return; }

	_openDate = today;

	SetFilePointer(handle_, 0, NULL, FILE_END);
}

void GSLog::Close()
{
	if (handle_ != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(handle_);
		handle_ = INVALID_HANDLE_VALUE;
	}
	_openDate = 0;
}

GSLog::~GSLog(void)
{
	EnterCriticalSection(&cs);
	Close();
	LeaveCriticalSection(&cs);

	DeleteCriticalSection(&cs);
}

bool			GSLog::Write(const char* fmt, ...)
{
	// 1) 포맷은 락 밖에서 (지역 버퍼만 사용)
	CHAR			arg_buffer[MAX_LOG_LEN];

	va_list ap;

	va_start(ap, fmt);
	_vsnprintf_s(arg_buffer, MAX_LOG_LEN, _TRUNCATE, fmt, ap);
	va_end(ap);

	SYSTEMTIME		sysTime;
	::GetLocalTime(&sysTime);

	CHAR			time_log[MAX_LOG_LEN + MAX_LOG_LEN];

	sprintf_s(time_log, MAX_LOG_LEN + MAX_LOG_LEN, "[ %04d-%02d-%02d %02d:%02d:%02d:%03d ] [PID=%lu] %s",
		sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute,
		sysTime.wSecond, sysTime.wMilliseconds, GetCurrentProcessId(), arg_buffer);

	DWORD	length = (DWORD)(strlen(time_log) * sizeof(CHAR));

	// 2) 파일 쓰기만 락 안에서
	bool result = true;

	EnterCriticalSection(&cs);

	Open();

	if (handle_ == INVALID_HANDLE_VALUE)
	{
		result = false;
	}
	else
	{
		DWORD	write_len = 0;
		result = ::WriteFile(handle_, time_log, length, &write_len, NULL) && write_len == length;
	}

	LeaveCriticalSection(&cs);

	// 3) 콘솔 출력은 락 밖에서 (printf는 자체적으로 스레드 안전)
	if (_display == true)
		printf(("%s\n"), time_log);

	return result;
}


extern GSLog &SYSLOG()
{
	static GSLog g_Log("system",false);
	return g_Log;
}
