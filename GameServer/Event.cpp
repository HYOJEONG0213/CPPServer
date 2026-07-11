#include "pch.h"
#include "Event.h"

void Event::Producer()
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

void Event::Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, [this]() { return q.empty() == false; }); // wait중인 스레드 깨우기
																// 조건문이 성립할때만 wait에서 빠져나오게 설정
																// 조건문 성립x -> 대기상태 유지

		{
			int32 data = q.front();
			q.pop();
			cout << data << "\n";
		}
	}
}