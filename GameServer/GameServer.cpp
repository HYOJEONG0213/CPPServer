#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#include "ThreadManager.h"
#include "CoreMacro.h"
#include "UserManager.h"
#include "AccountManager.h"

int main()
{
	GThreadManager->Launch(
		[=]
		{
			while (true)
			{
				cout << "UserThenAccount \n";
				GUserManager.UserThenAccount();
				this_thread::sleep_for(100ms);
			}
		});
	GThreadManager->Launch(
		[=]
		{
			while (true)
			{
				cout << "AccountThenPlayer \n";
				GAccountManager.AccountThenPlayer();
				this_thread::sleep_for(100ms);
			}
		});

	GThreadManager->Join();
}