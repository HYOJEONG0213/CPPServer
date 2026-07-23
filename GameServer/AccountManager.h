#pragma once
#include <mutex>

class Account
{
};

class AccountManager
{
	USE_LOCK;

public:
	void AccountThenPlayer();
	void Lock();
};

extern AccountManager GAccountManager;