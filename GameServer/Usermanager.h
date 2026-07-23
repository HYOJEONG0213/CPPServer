#pragma once
#include <mutex>

class User
{
};

class UserManager
{
	USE_LOCK;

public:
	void UserThenAccount();
	void Lock();
};

extern UserManager GUserManager;