#include "pch.h"
#include "system.h"
#include <thread>

#ifdef _MSC_VER
#include <windows.h>
using namespace std::this_thread;
#endif

namespace Core {

int64_t tickCount()
{
	auto tick = (int64_t)GetTickCount64();
	return tick;

}

int currentPid()
{
	return _getpid();
}
int currentTid()
{
	return (int)GetCurrentThreadId();
}

int lastError()
{
	return GetLastError();
}

void setThreadName(const string& threadName, int threadID)
{
	//UNUSED(dwThreadID);

	#ifdef _MSC_VER
	if (!IsDebuggerPresent())
	{
		return;
	}

	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName.c_str();
	info.dwThreadID = threadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
	#elif defined __APPLE__
	#else
	prctl(PR_SET_NAME, (unsigned long)szThreadName, 0, 0, 0);
	#endif

}


}