#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "ThreadManager.h"
#include "CoreMacro.h"

CoreGlobal Core;

void ThreadMain()
{
	while (true)
	{
		cout << "thread : " << LThreadId << "\n";
		this_thread::sleep_for(1s);
	}
}

int main()
{
	for (int32 i = 0; i < 5; i++) { GThreadManager->Launch(ThreadMain); }

	GThreadManager->Join();
}