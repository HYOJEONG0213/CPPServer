#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include "AccountManager.h"
#include "Usermanager.h"

void Func1()
{
	for (int32 i = 0; i < 100; i++) { UserManager::Instance()->ProcessSave(); }
}
void Func2()
{
	for (int32 i = 0; i < 100; i++) { AccountManager::Instance()->ProcessLogin(); }
}
int main()
{
	std::thread t1(Func1);
	std::thread t2(Func2);
	t1.join();
	t2.join();
	cout << "End!" << "\n";

	// 또다른 순서 맞추기 방법!
	mutex m1;
	mutex m2;
	std::lock(m1, m2); // 알아서 내부적으로 일관적 순서 정해 잠궈준다!

	// adopt_lock : 이미 잠겼으니 나중에 소멸될 때 풀어줘라!
	lock_guard<mutex> g1(m1, std::adopt_lock);
	lock_guard<mutex> g1(m2, std::adopt_lock);
}