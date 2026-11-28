#pragma once

/// @brief Windows CriticalSection을 수명 주기와 함께 관리하며 표준 잠금 가드 어댑터를 제공합니다.
class CGSLock
{

public:
	/// @brief 인스턴스의 CriticalSection을 초기화합니다.
	CGSLock()
	{
		InitializeCriticalSection(&mSync);
	}

	/// @brief 인스턴스의 CriticalSection을 해제합니다.
	~CGSLock()
	{
		DeleteCriticalSection(&mSync);
	}

	/// @brief CriticalSection의 소유권을 획득할 때까지 대기합니다.
	inline void Enter()
	{
		EnterCriticalSection(&mSync);
	}

	/// @brief 현재 스레드가 획득한 CriticalSection을 한 단계 해제합니다.
	inline void Leave()
	{
		LeaveCriticalSection(&mSync);
	}

	/// @brief 표준 잠금 인터페이스로 CriticalSection을 획득합니다.
	void lock() { Enter(); }
	/// @brief 표준 잠금 인터페이스로 CriticalSection을 해제합니다.
	void unlock() { Leave(); }
	/// @brief 대기하지 않고 CriticalSection 획득을 시도합니다.
	bool try_lock() { return TryEnterCriticalSection(&mSync) != FALSE; }

	CGSLock(const CGSLock&) = delete;
	CGSLock& operator=(const CGSLock&) = delete;

private:
	CRITICAL_SECTION	mSync;
};

