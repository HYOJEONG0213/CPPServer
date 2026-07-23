#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

AccountManager GAccountManager;

void AccountManager::AccountThenPlayer()
{
	WRITE_LOCK;

	this_thread::sleep_for(1s);

	GUserManager.Lock();
}

void AccountManager::Lock() { WRITE_LOCK; }