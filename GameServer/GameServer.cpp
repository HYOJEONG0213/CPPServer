#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>

void HelloThread() { cout << "thread hi\n"; };
void HelloThread2(int32 num) { cout << num << "\n"; };

int main()
{
	vector<std::thread> v;

	for (int32 i = 0; i < 10; i++) { v.push_back(std::thread(HelloThread2, i)); }
	for (int32 i = 0; i < 10; i++)
	{
		if (v[i].joinable()) v[i].join();
	}

	cout << "Hello Main \n";
}