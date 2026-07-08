#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>

vector<int32> v;
mutex		  m;

// RAII(Resource Acquisition Is Initialization)
// 라이 패턴(생성자에서 잠그고 소멸자에서 풀기)
template <typename T> class LockGuard
{
public:
	LockGuard(T &m)
	{
		_mutex = &m;
		_mutex->lock();
	}
	~LockGuard() { _mutex->unlock(); }

private:
	T *_mutex;
};

void Push()
{
	for (int32 i = 0; i < 10000; i++)
	{
		// 방법 1,2,3 : RAII 패턴을 이용하여 lock과 unlock을 자동으로 처리
		LockGuard<std::mutex> lockGuard(m); // 방법1 : 템플릿 만들기

		std::lock_guard<std::mutex> lockGuard(m); // 방법2 : STL 제공

		std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock); // 방법3 : STL 제공(옵션 추가 제공)
		uniqueLock.lock();											 // 방법3 : 수동으로 잠그고 풀기

		m.lock(); // 방법4 : 수동으로 잠그고 풀기
		v.push_back(i);
		m.unlock();
	}
}

int main()
{
	std::thread t1(Push);
	std::thread t2(Push);
	t1.join();
	t2.join();
	cout << v.size() << "\n";
}