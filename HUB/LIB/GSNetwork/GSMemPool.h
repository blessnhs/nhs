#pragma once

#include "Lock.h"

namespace GSNetwork	{	namespace GSMemPool	{	

template <class T, int ALLOC_BLOCK_SIZE = 500>
class GSMemPool	// [LOCKFIX] operator new/delete가 static이라 클래스 단위 static 락을 따로 둔다
{

public:
	static VOID* operator new(std::size_t allocLength)
	{
		PoolSync Sync;

		assert(sizeof(T) == allocLength);
		assert(sizeof(T) >= sizeof(UCHAR*));

		if (!mFreePointer)
			allocBlock();

		UCHAR *ReturnPointer = mFreePointer;
		mFreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);

		return ReturnPointer;
	}

	static VOID	operator delete(VOID* deletePointer)
	{
		PoolSync Sync;

		*reinterpret_cast<UCHAR**>(deletePointer) = mFreePointer;
		mFreePointer = static_cast<UCHAR*>(deletePointer);
	}

private:
	static VOID	allocBlock()
	{
		mFreePointer		= new UCHAR[sizeof(T) * ALLOC_BLOCK_SIZE];

		UCHAR **Current = reinterpret_cast<UCHAR **>(mFreePointer);
		UCHAR *Next		= mFreePointer;

		for (INT i=0;i<ALLOC_BLOCK_SIZE-1;++i)
		{
			Next		+= sizeof(T);
			*Current	= Next;
			Current		= reinterpret_cast<UCHAR**>(Next);
		}

		*Current = 0;
	}

private:
	static UCHAR	*mFreePointer;
	static CGSLock	mPoolSync;

	struct PoolSync
	{
		PoolSync()  { mPoolSync.Enter(); }
		~PoolSync() { mPoolSync.Leave(); }
	};

protected:
	~GSMemPool()
	{
	}
};

template <class T, int ALLOC_BLOCK_SIZE>
UCHAR* GSMemPool<T, ALLOC_BLOCK_SIZE>::mFreePointer;

template <class T, int ALLOC_BLOCK_SIZE>
CGSLock GSMemPool<T, ALLOC_BLOCK_SIZE>::mPoolSync;

}	}