#pragma once

#include <mutex>

template <typename T> class LockStack
{
public:
	LockStack() {}

	LockStack(const LockStack &) = delete;
	LockStack &operator=(const LockStack &) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one(); // 기다리고 있는 스레드가 있으면 깨워주기
	}

	bool TryPop(T &value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty()) return false;

		// c# 처럼 한번에 꺼내오면 c++에서는 move를 통해서 꺼내와야함(크래시 발생 가능)
		// 일반적으로는 원래 empty->top->pop() 순서로 호출하지만
		// 멀티스레드 환경에서는 다른 스레드가 pop()을 호출하므로 한번에 꺼내오기
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	// 멀티스레드 환경에선 empty()->pop() 사이에 다른 스레드가 pop()을 호출할 수 있음
	bool Empty()
	{
		lock_guard<mutex> lock(_mutex);
		return _stack.empty();
	}

	void WaitPop(T &value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _stack.empty() == false; }); // 데이터 있을때까지 대기
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T>		   _stack;
	mutex			   _mutex;
	condition_variable _condVar; // 기다려주기
};

template <typename T> class LockFreeStack
{
	struct Node
	{
		Node(const T &value) : data(value) {}

		T	  data;
		Node *next;
	};

public:
	void Push(const T &value)
	{
		// 새노드의 next를 현재 head로 설정 후 head를 새노드로 변경
		// 동시에 여러 스레드가 head에 접근시 문제 발생 가능
		Node *node = new Node(value);
		node->next = _head;

		while (_head.compare_exchange_weak(node->next, node) == false)
		{
			// 같을때까지 뺑뻉이 돌기
		}

		_head = node;
	}

	bool TryPop(T &value)
	{
		// head, head->next 읽은 후
		// 기존 head->next를 새 head로 설정 후 기존 head를 반환 및 삭제

		Node *oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			// 같을때까지 뺑뻉이 돌기
		}

		if (oldHead == nullptr) return false;

		value = oldHead->data;

		// 삭제시 다른 스레드에서 크래쉬가 날 수 있다! (삭제된 메모리에 접근하므로)
		// C#, Java 는 GC가 알아서 해줌;;
		// delete oldHead;
		return true;
	}

private:
	atomic<Node *> _head;
};
