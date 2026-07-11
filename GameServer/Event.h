#pragma once
#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include "windows.h"

class Event
{
public:
	void Producer();
	void Consumer();

	HANDLE handle;

private:
	mutex		 m;
	queue<int32> q;

	condition_variable cv; // 유저레벨 오브젝트!
};
