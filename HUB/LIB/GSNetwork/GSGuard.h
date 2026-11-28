#pragma once

#include "Lock.h"

// [LOCKFIX] 2026-11-26
// 이전: static CGSLock mSync  -> 같은 클래스(T)의 모든 객체가 락 하나를 공유(서버 전역 직렬화)
// 변경: 객체(인스턴스)마다 락 하나. 사용법: CThreadSync Sync(this);
// CRITICAL_SECTION 기반이라 같은 스레드의 재진입(중첩 잠금)은 계속 허용된다.
/// @brief 파생 객체별 CriticalSection과 스코프 기반 잠금 가드를 제공합니다.
template <class T>
class GSGuard
{
public:
	/// @brief 소유 객체의 잠금을 생성 시 획득하고 파괴 시 해제하는 가드입니다.
	class CThreadSync
	{
	public:
		/// @brief 대상 객체의 CriticalSection을 획득합니다.
		explicit CThreadSync(GSGuard<T>* Owner)
			: m_Owner(Owner)
		{
			m_Owner->mSync.Enter();
		}

		/// @brief 대상 객체의 CriticalSection을 해제합니다.
		~CThreadSync(VOID)
		{
			m_Owner->mSync.Leave();
		}

	private:
		/// @brief 대상 객체의 CriticalSection을 획득합니다.
		CThreadSync(const CThreadSync&);
		CThreadSync& operator=(const CThreadSync&);

		GSGuard<T>* m_Owner;
	};

	friend class CThreadSync;

	/// @brief 인스턴스마다 독립된 잠금을 준비합니다.
	GSGuard() {}
	// 객체를 복사해도 락은 복사하지 않고 새로 만든다 (CRITICAL_SECTION은 복사 불가)
	/// @brief 인스턴스마다 독립된 잠금을 준비합니다.
	GSGuard(const GSGuard&) {}
	GSGuard& operator=(const GSGuard&) { return *this; }

protected:
	/// @brief 인스턴스의 잠금을 정리합니다.
	~GSGuard() {}

private:
	CGSLock mSync;
};
