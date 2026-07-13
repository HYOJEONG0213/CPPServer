#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "SpinLock.h"
#include "Event.h"

class TestClass
{
public:
	int64 GetValue() { return 100; }
};

int64 Calculate()
{
	int64 sum = 0;
	for (int32 i = 0; i < 100000; i++) { sum += i; }

	return sum;
}

void PromiseWorker(std::promise<string> &&promise) { promise.set_value("Secret Message"); }

void TaskWorker(std::packaged_task<int64(void)> &&task) { task(); }

int main()
{
	{
		// std::future 를 사용한 비동기 방식!
		// 옵션 3개 (deferred, async, deferred | async)
		std::future<int64> future = std::async(std::launch::async, Calculate);

		std::future_status status = future.wait_for(1ms);
		if (status == future_status::ready) { cout << "ready \n"; }

		int64 sum = future.get(); // future.get() 은 Calculate() 가 끝날 때까지 기다림
	}

	{
		// 클래스 멤버 함수 호출하는 경우
		TestClass		   test;
		std::future<int64> future2 = std::async(std::launch::async, &TestClass::GetValue, test);
	}

	{
		// promise 를 사용한 비동기 방식!
		// 미래(std::future)에 결과물을 반환해줄거라는 약속하기
		std::promise<string> promise;
		std::future<string>	 future = promise.get_future(); // future : 메인 스레드

		thread t(PromiseWorker, std::move(promise)); // PromiseWorker : 워커 스레드

		string message = future.get();
		cout << message << "\n";

		t.join();
	}

	{
		// packaged_task 를 사용한 비동기 방식!
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64>				future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << "\n";

		t.join();
	}
}