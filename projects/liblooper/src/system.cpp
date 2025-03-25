#include "pch.h"
#include "system.h"
#include <thread>
#include "file.h"
#include <algorithm>

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

string appPathName()
{
	char buf[MAX_PATH];
	memset(buf, 0, sizeof(buf));

	#ifdef _MSC_VER
	::GetModuleFileNameA(NULL, buf, sizeof(buf) - 1);
	#else
	auto count = readlink("/proc/self/exe", buf, sizeof(buf) - 1);//fail in ios
	#ifdef __APPLE__
	//DV("count=%d", count);
	#endif
	if (count > 0)
	{
		ASSERT(count < sizeof(buf) - 1);
	}
	else {
		//DV("error=%d(%s)", errno, strerror(errno));//ios:error=2(No such file or directory)
	}
	#endif

	File::pathMakePretty(buf);

	return buf;
}

#ifdef _MSC_VER
static void fnAddress()
{
}
#endif

string appFolder()
{
	string pathname = appPathName();
	string path = File::parentFolder(pathname);

	#ifdef _MSC_VER
	{
		//在vs中做unit test时，我们一般希望app path为dll文件夹

		string testPath = path;
		transform(testPath.begin(), testPath.end(), testPath.begin(), toupper);

		if (testPath.find("/TESTPLATFORM") != string::npos)
		{
			HMODULE module = nullptr;
			auto ok = GetModuleHandleExA(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCSTR)fnAddress,
				&module
			);

			//DV("ok=%d,module=%p", module);
			if (ok)
			{
				char buf[MAX_PATH] = { 0 };
				GetModuleFileNameA(module, buf, sizeof(buf));
				path = File::parentFolder(buf);

				FreeLibrary(module);
				module = nullptr;
			}

		}
	}
	#endif

	return path;
}

}