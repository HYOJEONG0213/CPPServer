#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include "SpinLock.h"
#include "windows.h"

mutex		 m;
queue<int32> q;
HANDLE		 handle;

condition_variable cv; // 유저레벨 오브젝트!

void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait중인 스레드 하나만 깨우기
	}
}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() { return q.empty() == false; }); // wait중인 스레드 깨우기
															// 조건문이 성립할때만 wait에서 빠져나오게 설정
															// 조건문 성립x -> 대기상태 유지

		{
			int32 data = q.front();
			q.pop();
			cout << data << "\n";
		}
	}
}

int main()
{
	// [커널 오브젝트]
	// 이벤트 : 커널에서 관리하는 오브젝트
	// 프로세스끼리 동기화 필요할때도 사용 가능
	// Usage Count, Signal/Non-Signal(bool), Auto/Manual(bool)

	// ::CreateEvent : (보안속성, 수동/자동 리셋(FALSE: AUTO RESET), 초기상태, 이름)
	handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}