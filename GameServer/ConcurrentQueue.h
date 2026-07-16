#pragma once

#include <mutex>;

template <typename T> class LockQueue
{
public:
	LockQueue() {}
	LockQueue(const LockQueue &) = delete;
	LockQueue &operator=(const LockQueue &) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_condVar.notify_one(); // 기다리고 있는 스레드가 있으면 깨워주기
	}

	bool TryPop(T &value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_queue.empty()) return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	// 멀티스레드 환경에선 empty()->pop()을 호출하는 사이에 다른 스레드가 pop()을 호출할 수 있음
	bool Empty()
	{
		lock_guard<mutex> lock(_mutex);
		return _queue.empty();
	}

	void WaitPop(T &value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _queue.empty() == false; }); // 데이터 있을때까지 대기
		value = std::move(_queue.front());
		_queue.pop();
	}

private:
	queue<T>		   _queue;
	mutex			   _mutex;
	condition_variable _condVar;
};