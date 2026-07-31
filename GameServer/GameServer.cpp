#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "ThreadManager.h"
#include "CoreMacro.h"

// 멀티스레드를 사용해 100만 이하의 소수 개수 구하기 : 정답 : 78498
// 만 이하의 소수 개수 : 1229
// 천 이하의 소수 개수 : 168
atomic<int> ret;
const int	MAX_NUMBER = 1000000;

bool IsPrime(int num)
{
	if (num <= 1) return false;
	if (num == 2 || num == 3) return true;

	for (int i = 2; i * i <= num; i++)
	{
		if ((num % i) == 0) return false;
	}

	return true;
}

int f1(int s, int e)
{
	int count = 0;
	for (int i = s; i <= e; i++)
	{
		if (IsPrime(i)) count++;
	}
	return count;
}

int main()
{
	vector<thread> threads;

	int coreCount = thread::hardware_concurrency();
	int jobCount = (MAX_NUMBER / coreCount) + 1;

	for (int i = 0; i < coreCount; i++)
	{
		int start = (i * jobCount) + 1;
		int end = min(MAX_NUMBER, ((i + 1) * jobCount));

		threads.push_back(thread(thread([start, end]() { ret += f1(start, end); })));
	}

	for (thread &t : threads) { t.join(); }

	cout << ret << "\n";
}