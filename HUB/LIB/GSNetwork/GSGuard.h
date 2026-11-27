#pragma once

#include "Lock.h"

// [LOCKFIX] 2026-11-26
// 이전: static CGSLock mSync  -> 같은 클래스(T)의 모든 객체가 락 하나를 공유(서버 전역 직렬화)
// 변경: 객체(인스턴스)마다 락 하나. 사용법: CThreadSync Sync(this);
// CRITICAL_SECTION 기반이라 같은 스레드의 재진입(중첩 잠금)은 계속 허용된다.
template <class T>
class GSGuard
{
public:
	class CThreadSync
	{
	public:
		explicit CThreadSync(GSGuard<T>* Owner)
			: m_Owner(Owner)
		{
			m_Owner->mSync.Enter();
		}

		~CThreadSync(VOID)
		{
			m_Owner->mSync.Leave();
		}

	private:
		CThreadSync(const CThreadSync&);
		CThreadSync& operator=(const CThreadSync&);

		GSGuard<T>* m_Owner;
	};

	friend class CThreadSync;

	GSGuard() {}
	// 객체를 복사해도 락은 복사하지 않고 새로 만든다 (CRITICAL_SECTION은 복사 불가)
	GSGuard(const GSGuard&) {}
	GSGuard& operator=(const GSGuard&) { return *this; }

protected:
	~GSGuard() {}

private:
	CGSLock mSync;
};
