#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "SpinLock.h"
#include "Event.h"
#include "ConcurrentStack.h"
#include "ConcurrentQueue.h"

LockQueue<int32>				  q;
LockFreeStack<int32>			  s;
LockFreeStack_SmartPointer<int32> s2;

void Push()
{
	while (true)
	{
		int32 value = rand() % 100;
		s2.Push(value);

		this_thread::sleep_for(10ms);
	}
}

void Pop()
{
	while (true)
	{
		// int32 data = 0;
		// if (q.TryPop(OUT data)) cout << data << "\n";

		auto data = s2.TryPop();
		if (data != nullptr) cout << (*data) << "\n";
	}
}

int main()
{
	thread t1(Push);
	thread t2(Pop);
	thread t3(Pop);

	t1.join();
	t2.join();
	t3.join();
}