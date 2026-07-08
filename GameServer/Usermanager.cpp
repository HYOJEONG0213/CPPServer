#include "pch.h"
#include "Usermanager.h"
#include "AccountManager.h"

void UserManager::ProcessSave()
{
	Account *account = AccountManager::Instance()->GetAccount(100); // accountLock

	lock_guard<mutex> guard(_mutex); // userLock
}