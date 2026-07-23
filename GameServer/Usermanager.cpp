#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

UserManager GUserManager;

void UserManager::UserThenAccount()
{
	WRITE_LOCK;

	this_thread::sleep_for(1s);

	GAccountManager.Lock();
}

void UserManager::Lock() { WRITE_LOCK; }