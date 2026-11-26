#pragma once

class CGSLock
{

public:
	CGSLock()
	{
		InitializeCriticalSection(&mSync);
	}

	~CGSLock()
	{
		DeleteCriticalSection(&mSync);
	}

	inline void Enter()
	{
		EnterCriticalSection(&mSync);
	}

	inline void Leave()
	{
		LeaveCriticalSection(&mSync);
	}

	void lock() { Enter(); }
	void unlock() { Leave(); }
	bool try_lock() { return TryEnterCriticalSection(&mSync) != FALSE; }

	CGSLock(const CGSLock&) = delete;
	CGSLock& operator=(const CGSLock&) = delete;

private:
	CRITICAL_SECTION	mSync;
};

