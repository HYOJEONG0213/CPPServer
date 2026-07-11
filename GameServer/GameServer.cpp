#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include "SpinLock.h"
#include "windows.h"
#include "Event.h"

Event event;

int main()
{
	// ::CreateEvent : (보안속성, 수동/자동 리셋(FALSE: AUTO RESET), 초기상태, 이름)
	event.handle = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	thread t1(&Event::Producer, &event);
	thread t2(&Event::Consumer, &event);

	t1.join();
	t2.join();

	::CloseHandle(event.handle);
}