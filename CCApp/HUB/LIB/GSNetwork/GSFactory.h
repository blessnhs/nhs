#pragma  once
#include <new>
#include <boost/shared_ptr.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>
#include <boost/bind.hpp>


template<typename T,bool MultiThread = false>
class GSFactory 
{
public:
	GSFactory()
	{
		m_AllocCount = 0;
	}

	~GSFactory() 
	{
	}

	boost::shared_ptr<T> Create()
	{
		return alloc();
	}

	boost::shared_ptr<T> Malloc()
	{
		return alloc();		// [LOCKFIX] 이전: return Malloc(); (무한 재귀)
	}

	boost::shared_ptr<T> alloc()
	{
		m_AllocCount++;

		return boost::shared_ptr<T> (
			m_allocator.construct(),
			boost::bind(&GSFactory::Destroy,this,_1));
	}

private:

	void Destroy(T *point)
	{
		m_allocator.destroy(point);

		m_AllocCount--;
	}

	boost::object_pool<T> m_allocator;
	int  m_AllocCount;
};

// [LOCKFIX] 2026-11-26 멀티스레드용 팩토리
//  - 이전: GSGuard<GSFactory> 정적(전역) 락 + boost::object_pool
//          * Create() -> alloc() 이중 잠금
//          * Malloc() 무한 재귀
//          * object_pool::destroy()는 ordered_free()라서 free list 길이에 비례하는 O(N) 탐색을 락 안에서 수행
//          * T의 소멸자가 락을 잡은 채 실행됨 (소멸자 체인에서 다른 락을 잡으면 순서 역전 위험)
//  - 변경: 팩토리 객체마다 SRWLOCK 하나, boost::pool<>의 O(1) free 사용,
//          생성자/소멸자는 락 밖에서 실행하고 락은 메모리 블록을 얻고 돌려줄 때만 잡는다.
//  - 주의: boost::pool<>은 해제되지 않은 객체의 소멸자를 풀 소멸 시 호출하지 않는다
//          (프로세스 종료 시 남아 있는 객체의 소멸자는 실행되지 않음).
template<typename T>
class GSFactory<T,true>
{
public:
	GSFactory()
		: m_allocator(sizeof(T))
		, m_AllocCount(0)
	{
		InitializeSRWLock(&m_Lock);
	}

	~GSFactory() 
	{
	}

	boost::shared_ptr<T> Create()
	{
		return alloc();
	}

	boost::shared_ptr<T> Malloc()
	{
		return alloc();
	}

	boost::shared_ptr<T> alloc()
	{
		void* mem = NULL;

		AcquireSRWLockExclusive(&m_Lock);
		mem = m_allocator.malloc();
		if (mem != NULL)
			++m_AllocCount;
		ReleaseSRWLockExclusive(&m_Lock);

		if (mem == NULL)
			throw std::bad_alloc();

		T* obj = NULL;
		try
		{
			obj = new (mem) T();
		}
		catch (...)
		{
			Release(mem);
			throw;
		}

		// shared_ptr 제어 블록 할당이 실패하면 boost가 deleter(Destroy)를 호출해 준다.
		return boost::shared_ptr<T> (obj, boost::bind(&GSFactory::Destroy, this, _1));
	}

	int GetAllocCount()
	{
		AcquireSRWLockShared(&m_Lock);
		int count = m_AllocCount;
		ReleaseSRWLockShared(&m_Lock);
		return count;
	}

private:
	GSFactory(const GSFactory&);
	GSFactory& operator=(const GSFactory&);

	void Destroy(T *point)
	{
		if (point == NULL)
			return;

		point->~T();		// 락 밖에서 소멸자 실행
		Release(point);
	}

	void Release(void* mem)
	{
		AcquireSRWLockExclusive(&m_Lock);
		m_allocator.free(mem);	// 정렬하지 않는 free -> O(1)
		--m_AllocCount;
		ReleaseSRWLockExclusive(&m_Lock);
	}

	SRWLOCK					m_Lock;
	boost::pool<>			m_allocator;
	int						m_AllocCount;
};
