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
		Node(const T &value) : data(value), next(nullptr) {}

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
		_popCount++;

		// head, head->next 읽은 후
		// 기존 head->next를 새 head로 설정 후 기존 head를 반환 및 삭제

		Node *oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			// 같을때까지 뺑뻉이 돌기
		}

		if (oldHead == nullptr)
		{
			_popCount--;
			return false;
		}

		value = oldHead->data;

		TryDelete(oldHead);

		// 삭제시 다른 스레드에서 크래쉬가 날 수 있다! (삭제된 메모리에 접근하므로)
		// C#, Java 는 GC가 알아서 해줌;;
		// delete oldHead;
		return true;
	}

	void TryDelete(Node *oldHead)
	{
		if (_popCount == 1)
		{
			// 나 혼자 및 삭제 예약된 다른 데이터들도 삭제하기

			Node *node = _pendingList.exchange(nullptr);

			if (--_popCount == 0)
			{
				// 중간에 끼어든 얘가 없다면 그냥 삭제 진행
				// 이제와서 끼어들어도 데이터는 분리해둔 상태
				DeleteNodes(node);
			}
			else if (node)
			{
				// 중간에 끼어든 얘가 있다면 다시 갖다놓기
				ChainPendingNodeList(node);
			}

			delete oldHead;
		}
		else
		{
			// 이미 누군가가 있다면 삭제만 예약하자
			ChainPeningNode(oldHead);
			_popCount--;
		}
	}

	void ChainPendingNodeList(Node *first, Node *last)
	{
		// 데이터 갖다놓기 (pendingList를 Node 뒤에 붙이기)
		last->next = _pendingList;

		// 이러는 와중 또 끼어들을 수 있으므로 CAS 필요
		while (_pendingList.compare_exchange_weak(last->next, first) == false) {}
	}

	void ChainPendingNodeList(Node *node)
	{
		// _pendingList 의 마지막 노드 찾기
		Node *last = node;
		while (last->next) last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node *node)
	{
		// 한개만 붙이는 경우
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node *node)
	{
		while (node)
		{
			Node *next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node *> _head;
	atomic<uint32> _popCount = 0; // pop을 실행중인 스레드 개수 카운팅
	atomic<Node *> _pendingList;  // 삭제되어야 할 노드들(첫번째 노드)
};
