#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock()
{
	// 동일한 스레드가 소유하고 있다면 무조건 성공해야함
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16; // 상위 16비트 추출
	if (LThreadId == lockThreadId)
	{
		// 동일한 스레드에서 또 write lock 건 것
		_writeCount++;
		return;
	}

	// 아무도 소유, 공유하고 있지 않을때(모두 0) 경합해 소유권 얻기
	const int64	 beginTick = ::GetTickCount64(); // 오래 Spinlock 시 크래시 발생시키기
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// if 경합해서 이긴 경우
				_writeCount++;
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK) CRASH("LOCK_TIMEOUT");

		this_thread::yield(); // 소유권 내려놓기
	}
}

void Lock::WriteUnlock()
{
	// ReadLock 다 풀기 전에는 WriteUnlock 불가능.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0) _lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock()
{
	// 동일한 스레드가 소유시 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하지 않을때 경합해 공유카운트 올리기
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);					   // write 부분 밀기
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1)) return; // 리드락 잡기 성공
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK) CRASH("LOCK_TIMEOUT");

		this_thread::yield();
	}
}

void Lock::ReadUnlock()
{
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0) CRASH("MULTIPLE_UNLOCK");
}
