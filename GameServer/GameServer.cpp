#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock()
	{
		// CAS (Compare And Swap) : 원자적 연산
		bool expected = false;
		bool desired = true;

		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			// expected 는 매번 바뀌므로 계속 초기화 필요
			expected = false;

			// Sleep 구현 : 세 방법 중 골라쓰기
			this_thread::sleep_for(std::chrono::milliseconds(100)); // 100ms간 쉬고 다시 시도
			this_thread::sleep_for(100ms);
			this_thread::yield(); // 타임슬라이스 양보 후 커널모드로 돌아가 알아서 스케줄링하라고 떠넘기기
								  // = sleep_for(0ms)
		};
	}
	void unlock()
	{
		_locked.store(false); //_locked=false; 해도 되지만 atomic이므로 명시적으로 store로 한거임!
	}

private:
	atomic<bool> _locked = false;
};

int32	 sum = 0;
mutex	 m;
SpinLock spinLock;

void Add()
{
	for (int32 i = 0; i < 100000; i++)
	{
		lock_guard<SpinLock> guard(spinLock);
		sum++;
	};
}

void Sub()
{
	for (int32 i = 0; i < 100000; i++)
	{
		lock_guard<SpinLock> guard(spinLock);
		sum--;
	};
}

int main()
{
	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();

	cout << sum << "\n";
}