#include "pch.h"
#include "DeadLockProfiler.h"

// DeadLock 탐지

void DeadLockProfiler::PushLock(const char *name)
{
	LockGuard guard(_lock);
	int32	  lockId = 0;
	auto	  findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) // 없다면 등록
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second; // id 추출
	}

	// 잡고있는 락이 있다면
	if (LLockStack.empty() == false)
	{
		// 기존에 발견되지 않았다면 데드락 여부 확인
		const int32 prevId = LLockStack.top();
		if (lockId != prevId)
		{
			set<int32> &history = _lockHistory[prevId];
			if (history.find(lockId) == history.end())
			{
				// 처음 발견시
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char *name)
{
	LockGuard gurad(_lock);

	if (LLockStack.empty()) CRASH("MULTIPLE_UNLOCK");

	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId) CRASH("INVALID_UNLOCK");

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	// 초기화
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	// 사이클 여부 체크
	for (int32 lockId = 0; lockId < lockCount; lockId++) { Dfs(lockId); }

	// 정리
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1) return;
	_discoveredOrder[here] = _discoveredCount++;

	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		// 해당 정점을 lock으로 잡은 상태에서 다른 lock을 잡은 적 없음
		_finished[here] = true;
		return;
	}

	// 내가 lock을 잡은 뒤 다른 lock을 잡은 적 있음 -> 사이클 확인 필요
	set<int32> &nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 방문한적 있었다면 순방향, 역방향 확인 필요
		// here가 먼저 발견되었다면 here은 조상임 (순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there]) continue;

		// 순방향이 아니며 Dfs(there)가 아직 종료되지 않았다면 there은 here의 조상 (역방향 간선)
		// Dfs가 끝나지 않은 상태에서 역으로 가는길 발견!
		if (_finished[there] == false)
		{
			printf("%s -> %s \n", _idToName[here], _idToName[there]);
			int32 now = here;
			while (true)
			{
				printf("%s -> %s \n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there) break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
