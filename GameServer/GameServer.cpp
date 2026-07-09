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

		// _locked==expected라면 expected = _locked, _locked = desired로 바꾸고 true 리턴 : (1)
		// _locked!=expected라면 expected = _locked, false 리턴 : (2)

		// _locked==false면 (1) 실행 후 잠긴상태로 만들고 true 리턴
		// _locked==true면 (2) 실행 후 while문으로 빠져나오지 못하고 계속 반복
		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			// expected 는 매번 바뀌므로 계속 초기화 필요
			expected = false;
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