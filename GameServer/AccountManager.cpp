#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
	// 순서가 뒤바뀌면 여러 lock이 동시에 잡게되면서 deadlock이 발생 가능
	lock_guard<mutex> guard(_mutex);	   // accountLock
	UserManager::Instance()->GetUser(100); // userLock
}