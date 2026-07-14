#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "SpinLock.h"
#include "Event.h"

atomic<bool> flag;
atomic<bool> ready;
int32		 value;

void Producer()
{
	value = 10;
	// 타입1
	ready.store(true, memory_order::memory_order_release);
	// -------- 절취선 -------

	// 타입2
	ready.store(true);
	std::atomic_thread_fence(memory_order::memory_order_release)
}

void Consumer()
{
	// -------- 절취선 -------
	while (ready.load(memory_order::memory_order_acquire) == false) { ; }

	cout << value << "\n";
}

int main()
{
	// 복습용 코드
	{
		{
			flag = false;

			flag.store(true, memory_order::memory_order_seq_cst);

			bool val = flag.load(memory_order::memory_order_seq_cst);
		}

		{
			// 아래처럼 하면  멀티 스레드에서 꼬일수있음(원자적으로 이뤄지지 않음)
			// .exchange() 를 사용하자!
			// bool prev = flag;
			// flag = true;

			bool prev = flag.exchange(true);
		}

		{
			// CAS(Compare and Swap) 조건부 수정
			bool expected = false;
			bool desired = true;
			flag.compare_exchange_strong(expected, desired); // expected가 false이면 desired로 바꾸고
															 // 아니면 expected를 현재 flag값으로 바꿈

			// flag.compare_exchange_weak(expected, desired); // weak : Spurious Failure(가짜 실패) 가능성 있음
		}
	}

	// Memory Model (정책)
	// Sequentially Consistent (seq_cst)
	// Acquire Release (consume, acquire, release, acq_rel)
	// Relaxed (relaxed)

	ready = false;
	value = 0;
	thread t1(Producer);
	thread t2(Consumer);
	t1.join();
	t2.join();
}