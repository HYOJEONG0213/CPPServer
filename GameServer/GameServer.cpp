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

void Producer()
{
	while (true)
	{
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		::SetEvent(handle); // Signal 상태로 변경

		this_thread::sleep_for(10000000ms);
	}
}

void Consumer()
{
	while (true)
	{
		::WaitForSingleObject(handle, INFINITE); // Signal 상태가 될 때까지 대기
		//::ResetEvent(handle);					 // Non-Signal 상태로 변경

		unique_lock<mutex> lock(m);
		if (q.empty() == false)
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