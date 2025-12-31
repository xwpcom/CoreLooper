#include "stdafx.h"
#include "base/shelltool.h"
#include "bytetool.h"
#include "string/textseparator.h"
#include "base/stringtool.h"
#include "file/inifile.h"
#include "core/net/nettool.h"
#include "string/utf8tool.h"
#include <time.h>
#ifdef _MSC_VER
using namespace Bear::Core::Net;
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#include <Mmsystem.h>
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Rpcrt4.lib")
#include <sys/timeb.h>
#else
#include <ctype.h>
#include <pthread.h>
#include <sys/types.h>
#ifndef __APPLE__
#include <sys/prctl.h>	//support for SetThreadName
#include <sys/reboot.h>
#endif
#include <sys/syscall.h>
#ifndef __APPLE__
#include <sys/prctl.h>
#endif
#endif

#ifdef _CONFIG_ANDROID
#include "arch/android/jnihelper.h"
#endif

static const char* TAG = "ShellTool";

#ifdef __APPLE__
#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

long GetTickCount_ios()
{
	//ios
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	//ts.tv_sec = mts.tv_sec;
	//ts.tv_nsec = mts.tv_nsec;
	
	long dwTick = mts.tv_sec * 1000 + mts.tv_nsec/1000000;
	static long firstTick=0;
	if(firstTick==0)
	{
		firstTick=dwTick;
	}
	
	return dwTick-firstTick;
}

#endif

using namespace std;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;

#if defined _MSC_VER || defined _CONFIG_HI3516|| defined _CONFIG_HI3519 || defined _CONFIG_ANDROID || defined _CONFIG_INGENIC
//hi3516不支持 undefined reference to `backtrace'
int backtrace(void **buffer, int size) { return 0; }
char **backtrace_symbols(void *const *buffer, int size) { return nullptr; }
void backtrace_symbols_fd(void *const *buffer, int size, int fd) {}
#else
#include <execinfo.h>//hi3516 toolchain没有此头文件
#endif

namespace Bear {
namespace Core {

void DumpCallStackX()
{
	int j, nptrs;
#define SIZE 100
	void* buffer[100];
	char** strings;

	nptrs = backtrace(buffer, SIZE);
	//printf("backtrace() returned %d addresses\n", nptrs);

	/* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
	would produce similar output to the following: */

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL)
	{
		//perror("backtrace_symbols");
		return;
	}

	for (j = 0; j < nptrs; j++)
		printf("[%02d]%s\n", j, strings[j]);

	free(strings);
}

}
}

DWORD ShellTool::GetTickCount()
{
	{
		static bool first = true;
		if (first)
		{
			first = false;
			LogW(TAG, "please use GetTickCount64()");
		}
	}

#ifdef _MSC_VER
	return ::timeGetTime();
#elif defined __APPLE__
	return (DWORD)GetTickCount_ios();
#else
#ifdef _CONFIG_XM3518
	//2017.07.24,现在xm3518上面没有librt.so,所以不能用clock_gettime
	struct timeval tv = { 0 };
	int ret=gettimeofday(&tv,nullptr);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
	//XiongWanPing 2009.12.10
	//已通过修改系统时间测试clock_gettime,它返回的tick是单调增加的,与系统时间完全无关
	struct timespec tp;
	tp.tv_sec = 0;
	tp.tv_nsec = 0;

	auto flag = CLOCK_MONOTONIC;
#ifdef CLOCK_MONOTONIC_RAW
	flag = CLOCK_MONOTONIC_RAW;
#endif
	long ret = clock_gettime(flag, &tp);
	if (ret)
	{
		LogW(TAG,"err=%d(%s)", errno, strerror(errno));
	}
	ASSERT(ret == 0);
	DWORD dwTick = (DWORD)(tp.tv_sec * 1000) + (DWORD)(tp.tv_nsec / 1000000);
	return dwTick;
#endif
#endif
}

ULONGLONG ShellTool::GetTickCount64()
{
#ifdef _MSC_VER
	ULONGLONG tick = ::GetTickCount64();
	return tick;
#elif defined __APPLE__
	return (ULONGLONG)GetTickCount_ios();
#else

#ifdef _CONFIG_ANDROID
	/*
		为了检查视频延时的原因,统一采用SystemClock.elapsedRealtime()标记frame tick
		这样java和c++ tick统一起来,便于排查是哪个环节出了问题
	*/
	if (0)
	{
		JavaVM* jvm = AfxGetJavaVM();
		if (jvm)
		{
			/*
			启用后可视对讲在a64上空跑也要占用25% cpu,所以仅在调试时使用本功能
			不启用由cpu占用为0%
			*/
			return JniHelper::SystemClock_elapsedRealtime();
		}
	}
#endif
	struct timespec tp;
	tp.tv_sec = 0;
	tp.tv_nsec = 0;

	auto flag = CLOCK_MONOTONIC;
#ifdef CLOCK_MONOTONIC_RAW
	flag = CLOCK_MONOTONIC_RAW;
#endif

	long ret = clock_gettime(flag, &tp);
	if (ret)
	{
		LogW(TAG,"err=%d(%s)", errno, strerror(errno));
	}
	ASSERT(ret == 0);
	ULONGLONG tick = (ULONGLONG)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

	//tick += 0xFFFFFFFF-60*1000;//故意超过32bit回绕的场景,test ok

	return tick;
#endif
}

int ShellTool::GetLastError()
{
#ifdef _MSC_VER
	int err = ::GetLastError();
	return err;
#else
	return errno;
#endif
}

void ShellTool::Sleep(UINT ms)
{
#ifdef _MSC_VER
	::Sleep(ms);
#else

#if defined _TARGET_AMBARELLA
	//XiongWanPing 2014.11.20 usleep太小时，在ambarella上会占用较高cpu
	if (ms<5)
	{
		ms = 5;
	}
#endif

	DWORD us = ms * 1000;
	const auto tickStart = ShellTool::GetTickCount64();
	while (1)
	{
		int ret = ::usleep(us);
		if (ret == 0)
		{
			break;
		}

		LogW(TAG,"usleep fail,err=%d,desc=[%s]", errno, strerror(errno));

		const auto tickNow = ShellTool::GetTickCount64();
		if (tickNow >= tickStart + ms)
		{
			break;
		}

		us = (DWORD)((tickStart + ms - tickNow) * 1000);
	}
	//DT("Sleep(%d),idx=%d",ms,idx);//在mt7620上测试Sleep(1000),idx=250,每次调用最小时间大约为4ms
#endif
}

#ifdef _MSC_VER

string ShellTool::CreateGuid()
{
	RPC_CSTR pszGuid = nullptr;
	UUID guid = { 0 };
	auto hr = UuidCreate(&guid);
	hr = UuidToStringA(&guid, &pszGuid);
	string szGuid = (char*)pszGuid;
	RpcStringFreeA(&pszGuid);
	pszGuid = NULL;

	return szGuid;
}

string ShellTool::CreateGuidToken()
{
	auto guid = ShellTool::CreateGuid();
	StringTool::MakeUpper(guid);
	StringTool::Replace(guid, "-", "");
	return guid;
}

struct tagTimeMs ShellTool::GetRelativeTimeMs(int deltaDays)
{
	CTime t = CTime::GetCurrentTime();
	t += CTimeSpan(deltaDays, 0, 0, 0);

	tagTimeMs obj;
	obj.year = t.GetYear();
	obj.month = t.GetMonth();
	obj.day = t.GetDay();
	obj.hour = t.GetHour();
	obj.minute = t.GetMinute();
	obj.second = t.GetSecond();
	obj.ms = 0;
	return obj;
}
#endif

struct tagTimeMs ShellTool::GetCurrentTimeMs()
{
	return tagTimeMs::now();
}

BOOL ShellTool::QueueUserWorkItem(LPTHREAD_START_ROUTINE Function, PVOID Context, ULONG Flags)
{
	//DT("%s",__func__);
	UNUSED(Flags);

	Flags = 0;

#ifdef _MSC_VER
	BOOL bOK = FALSE;
	bOK = ::QueueUserWorkItem(Function, Context, WT_EXECUTELONGFUNCTION);
	return bOK;
#else
	ASSERT(Flags == 0);//linux not support flags

	{
		pthread_t hThread = 0;
		int ret = pthread_create(&hThread, NULL, Function, Context);
		if (ret == 0)
		{
			pthread_detach(hThread);
		}
		else
		{
			LogW(TAG,"Fail pthread_create,error=%d(%s)",errno,strerror(errno));
			ASSERT(FALSE);
		}

		return ret == 0;
	}

#if 0
	//发现mips,winbond745采用下面的代码会crash,原因未知
	pthread_attr_t attr;
	pthread_t thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ret = pthread_create(&thread, &attr, Function, Context);
	pthread_attr_destroy(&attr);
	ASSERT(ret == 0);
	return ret == 0;
#endif

#endif
}

#if !defined _MSC_VER && !defined _CONFIG_ANDROID && !defined __APPLE__
#include <sys/epoll.h>
#include <sys/syscall.h>
//static pid_t gettid(void)
//{
//	return (pid_t)syscall(__NR_gettid);
//}
#endif

DWORD ShellTool::GetCurrentProcessId()
{
#ifdef _MSC_VER
	return ::GetCurrentProcessId();
#else
	return getpid();
#endif
}

DWORD ShellTool::GetCurrentThreadId()
{
	DWORD id=0;
#if defined _CONFIG_ANDROID
	//#error android
	id=gettid();
#elif defined _MSC_VER
	id=::GetCurrentThreadId();
#elif defined __APPLE__
	return pthread_mach_thread_np(pthread_self());
#else
	//man gettid上面说:
	//Glibc does not provide a wrapper for this system call; call it using syscall(2).
	//man syscall可查到syscall用法:
	pid_t tid;
	tid = (pid_t)syscall(SYS_gettid);
	id=(DWORD)tid;
#endif
	
	ASSERT(id);
	return id;
	
}

string ShellTool::GetAppPathName()
{
	char buf[MAX_PATH];
	memset(buf,0,sizeof(buf));

#ifdef _MSC_VER
	::GetModuleFileNameA(NULL, buf, sizeof(buf) - 1);
#else
	auto count = readlink("/proc/self/exe", buf, sizeof(buf) - 1);//fail in ios
#ifdef __APPLE__
	DV("count=%d",count);
#endif
	if(count>0)
	{
		ASSERT(count<sizeof(buf) - 1);
	}
	else{
		DV("error=%d(%s)",errno,strerror(errno));//ios:error=2(No such file or directory)
	}
#endif
	
	File::PathMakePretty(buf);

	return buf;
}

string ShellTool::GetAppDisk()
{
	string folder = ShellTool::GetAppPath().c_str();
	return folder.substr(0,2);
}

#ifdef _MSC_VER
static void fnAddress()
{
}
#endif

string ShellTool::GetAppPath()
{
	string pathname = GetAppPathName();
	string path = File::GetUpperFolder(pathname.c_str()).c_str();

#ifdef _MSC_VER
	{
		//在vs中做unit test时，我们一般希望app path为dll文件夹

		string testPath = path;
		StringTool::MakeUpper(testPath);
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
				//DV("buf=%s", buf);
				string pathname = buf;
				path = File::GetUpperFolder(pathname.c_str()).c_str();

				FreeLibrary(module);
				module = nullptr;
			}

		}
	}
#endif

	return path;
}

/*
string ShellTool::GetSharePath()
{
	char path[MAX_PATH] = { 0 };
	string name = "BEAR_SHARE";
#ifdef _MSC_VER
	GetEnvironmentVariable(name, path, sizeof(path));
#else
	char *str=getenv(name);
	return str;
#endif
	if (path[0] == 0)
	{
		return GetAppPath().c_str();
	}

	return path;
}
*/

GUID ShellTool::UuidFromString(string szGuid)
{
	//DT("UuidFromString(%s)",szGuid.c_str());
	GUID guid;

#ifdef _MSC_VER
	RPC_STATUS rs = ::UuidFromStringA((unsigned char *)szGuid.c_str(), &guid);
#else
	//"e87db3a0-109a-4ecb-8664-66854520a6cc";
	const char *psrc = szGuid.c_str();
	LPBYTE pdst = (LPBYTE)&guid;

	int ret = ByteTool::HexCharToByte(psrc, pdst, 4);
	UNUSED(ret);
	ASSERT(ret==0);
	pdst += 4;
	psrc += 9;

	for (int i = 0; i<3; i++)
	{
		ret = ByteTool::HexCharToByte(psrc, pdst, 2);
		ASSERT(ret==0);
		pdst += 2;
		psrc += 5;
	}

	ret= ByteTool::HexCharToByte(psrc, pdst, 6);
	ASSERT(ret==0);
	pdst += 6;
	psrc += 12;

	guid.Data1 = ntohl((uint32_t)guid.Data1);//maybe bug?
	guid.Data2 = (unsigned short)ntohs(guid.Data2);
	guid.Data3 = (unsigned short)ntohs(guid.Data3);
#endif

	return guid;
}

string ShellTool::GuidToString(GUID& guid)
{
	UNUSED(guid);

	ASSERT(FALSE);
	return "";

	/*
	unsigned char *pszGuid=NULL;
	UuidToString(&guid,&pszGuid);
	string szGuid=(char*)pszGuid;
	RpcStringFree(&pszGuid);
	pszGuid=NULL;
	szGuid.MakeUpper();
	return szGuid;
	//*/
}

#ifdef _MSC_VER
string ShellTool::RelativePath2FullPath(string relativePath)
{

	char szPath[MAX_PATH];
	memset(szPath, 0, sizeof(szPath));

	strncpy(szPath, relativePath.c_str(), sizeof(szPath) - 1);

	char fullPath[MAX_PATH];
	memset(fullPath, 0, sizeof(fullPath));
	int retval = GetFullPathNameA(szPath, (int)strlen(szPath), fullPath, NULL);
	if (strlen(fullPath) > 0)
	{
		strncpy(szPath, fullPath, sizeof(szPath) - 1);
	}

	return szPath;
}
#endif

void ShellTool::SetThreadName(LPCSTR szThreadName, DWORD dwThreadID)
{
	UNUSED(dwThreadID);

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
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
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

string ShellTool::Bytes2String(LPBYTE pBuf, int cbBuf)
{
	string text;
	for (int i = 0; i < cbBuf; i++)
	{
		text+=StringTool::Format("%02x ", pBuf[i]);
	}
	return text;
}

int ShellTool::PrintHex(LPBYTE pBuf, int cbBuf, const char *desc)
{
	string text;
	for (int i = 0; i < cbBuf; i++)
	{
		text+= StringTool::Format("%02x ", pBuf[i]);
	}
	DV("%s %s", desc,text.c_str());
/*
	printf("%s\n", desc);
	for (int i = 0; i<cbBuf; i++)
	{
		printf("%02x ", pBuf[i]);
	}
	printf("\n");
	*/

	return 0;
}

string ShellTool::Second2Str(int seconds)
{
	int days = seconds / (24 * 3600);
	seconds -= days * 24 * 3600;

	int nHour = seconds / 3600;				// Hour
	int nMin = (seconds % 3600) / 60;			// Min
	int nSecond = (seconds % 60);				// sec

	string text;
	if (days > 0)
	{
		text = StringTool::Format("%d %02d:%02d:%02d", days, nHour, nMin, nSecond);
	}
	else
	{
		text =StringTool::Format("%02d:%02d:%02d", nHour, nMin, nSecond);
	}

	return text;
}

struct tm ShellTool::GetLocalTime(int deltaSecond)
{
	time_t time1;
	time(&time1);
	time1 += deltaSecond;

	struct tm tmNow;
	memset(&tmNow,0, sizeof(tmNow));
#ifdef _MSC_VER
	{
		struct tm *p = localtime(&time1);
		if (p)
		{
			tmNow = *p;
		}
	}
#else
	localtime_r(&time1, &tmNow);
#endif

	return tmNow;
}

#ifdef _MSC_VER
void ShellTool::SaveWindowPosHelper(HWND hWnd, CString szWinName, BOOL bSave, BOOL bShow)
{
	if (bSave)
	{
		// save windows position info
		WINDOWPLACEMENT wndStatus = { 0 };
		wndStatus.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &wndStatus);
		AfxGetApp()->WriteProfileBinary(_T("Settings"), szWinName,(LPBYTE)&wndStatus, sizeof(wndStatus));
	}
	else
	{
		unsigned char * pData = NULL;
		UINT nRead = 0;
		WINDOWPLACEMENT wndStatus = { 0 };
		AfxGetApp()->GetProfileBinary(_T("Settings"), szWinName,&pData, &nRead);
		if (nRead == sizeof(wndStatus))
		{
			memcpy((char*)&wndStatus, (char*)pData, nRead);
			if (!bShow)
			{
				wndStatus.showCmd = SW_HIDE;
			}
			VERIFY(SetWindowPlacement(hWnd, &wndStatus));
		}
		else
		{
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		}

		delete pData;
		pData = NULL;
	}
}

void ShellTool::SaveWindowPosHelper(HWND hWnd, CString szWinName, BOOL bSave, IniFile& ini)
{
	if (bSave)
	{
		// save windows position info
		WINDOWPLACEMENT wndStatus = { 0 };
		wndStatus.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &wndStatus);

		auto sz=ByteTool::ByteToHexChar((LPBYTE)&wndStatus, sizeof(wndStatus));
		//AfxGetApp()->WriteProfileBinary(_T("Settings"), szWinName, (LPBYTE)&wndStatus, sizeof(wndStatus));
		USES_CONVERSION;
		ini.SetString(T2A(szWinName),"windowPos",sz.c_str());
	}
	else
	{
		unsigned char * pData = NULL;
		UINT nRead = 0;
		WINDOWPLACEMENT wndStatus = { 0 };
		//AfxGetApp()->GetProfileBinary(_T("Settings"), szWinName, &pData, &nRead);
		USES_CONVERSION;
		auto sz = ini.GetString(T2A(szWinName), "windowPos"); 
		if (sz.length() == sizeof(wndStatus) * 2)
		{
			ByteTool::HexCharToByte(sz.c_str(), (LPBYTE)&wndStatus, sizeof(wndStatus));
			//memcpy((char*)&wndStatus, (char*)pData, nRead);
			VERIFY(SetWindowPlacement(hWnd, &wndStatus));
			//ByteTool::ByteToHexChar((LPBYTE)&wndStatus, sizeof(wndStatus));
		}
		//AfxGetApp()->WriteProfileBinary(_T("Settings"), szWinName, (LPBYTE)&wndStatus, sizeof(wndStatus));
		else
		{
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		}

		//delete pData;
		//pData = NULL;
	}
}

LPWSTR *ShellTool::CommandLineToArgvW(LPCWSTR lpCmdLine, int *pNumArgs)
{
	return ::CommandLineToArgvW(lpCmdLine, pNumArgs);
}

void ShellTool::SaveWindowPos(HWND hWnd, CString szWinName)
{
	SaveWindowPosHelper(hWnd, szWinName, TRUE);
}

void ShellTool::LoadWindowPos(HWND hWnd, CString szWinName, BOOL bShow)
{
	SaveWindowPosHelper(hWnd, szWinName, FALSE, bShow);
}

void ShellTool::SaveWindowPos(HWND hWnd, CString szWinName, IniFile& ini)
{
	SaveWindowPosHelper(hWnd, szWinName, true, ini);
}

void ShellTool::LoadWindowPos(HWND hWnd, CString szWinName, IniFile& ini)
{
	SaveWindowPosHelper(hWnd, szWinName, false,ini);
}

//有些子线程需要向主线程SendMessage,
//如果在主线程中调用WaitForSingleObject等待这些线程结束,而子线程恰好调用了SendMessage,则会形成死锁.
//为此,可以调用WaitThreadExitEx,WaitThreadExitEx在等待子线程状态变化的同时能处理主线程中的消息.
//WaitThreadExitEx能过滤等待过程中的用户输入,如mouse,keyboard.
BOOL ShellTool::WaitThreadExitEx(HANDLE handle)
{
	//  LogW(TAG,"WaitThreadExitEx#1,threadid=%d",GetCurrentThreadId());

	while (TRUE)
	{
		DWORD result;
		MSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
				|| (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
				)
			{
				//skip user input
			}
			else
			{
				DispatchMessage(&msg);
			}
		}

		HANDLE *lphObjects = &handle;
		DWORD cObjects = 1;
		// Wait for any message sent or posted to this queue or for one of the passed handles be set to signaled.
		result = MsgWaitForMultipleObjects(cObjects, lphObjects, FALSE, INFINITE,
			//QS_ALLINPUT
			QS_SENDMESSAGE
			| QS_TIMER
			| QS_PAINT
			| QS_ALLPOSTMESSAGE

		);

		// The result tells us the type of event we have.
		if (result == (WAIT_OBJECT_0 + cObjects))
		{
			//DT(" New messages have arrived. Dispatch message");
			// Continue to the top of the always while loop to 
			// dispatch them and resume waiting.
			continue;
		}
		else
		{
			{
				ASSERT(AfxGetMainWnd());
			}


			//LogW(TAG,"One of the handles became signaled.,exit MessageLoop");
			return TRUE;
			//DoStuff (result - WAIT_OBJECT_0) ; 
		} // End of else clause.
	}
	//LogW(TAG,"WaitThreadExitEx#2,threadid=%d",GetCurrentThreadId());
}
//进行message loop循环的同时等待某个变量值为FALSE.
//一般用于确保工作线程事务完成.
//xwpcom 2009.07.23
void ShellTool::DispatchMessageWaitFalse(BOOL& bNeedWait, HWND hWndTempDisable)
{
	if (hWndTempDisable == NULL && AfxGetMainWnd())
		hWndTempDisable = AfxGetMainWnd()->m_hWnd;

	if (hWndTempDisable)
		::EnableWindow(hWndTempDisable, FALSE);

	BOOL& bValue = bNeedWait;
	if (bValue)
	{
		//wait search thread to exit
		{
			MSG msg;
			while (bValue)
			{
				BOOL bHasMsg = FALSE;

				//XiongWanPing 2013.07.12
				//CWaitCursor内部调用了AfxGetApp(),在firefox/chrome环境下为NULL,会导致crash
				//CWaitCursor wc;

				while (bValue && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
						|| (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
						//|| (msg.message== WM_SETCURSOR )
						)
					{
						//skip user input
					}
					else
					{
						bHasMsg = TRUE;
						DispatchMessage(&msg);
					}
				}
				if (!bHasMsg)
				{
					Sleep(1);
				}
			}
		}

		ASSERT(!bValue);
	}

	if (hWndTempDisable)
		::EnableWindow(hWndTempDisable, TRUE);
}

void ShellTool::EatUpMessage(BOOL bDiscardUserOp)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, WM_USER, 0, PM_REMOVE))
	{
		if (bDiscardUserOp)
		{
			if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
				|| (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
				|| (msg.message >= WM_NCMOUSEMOVE && msg.message <= WM_NCMBUTTONDBLCLK)
				)
			{
				continue;
			}

		}

		DispatchMessage(&msg);
	}
}

#endif

#ifdef __AFX_H__
void ShellTool::OnInitMenuPopupHelper(CWnd *pWnd, CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(pWnd->m_hWnd)) != NULL)
	{
		CWnd* pParent = pWnd;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(pWnd, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(pWnd, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}
#endif

string ShellTool::GetComputerName()
{
#ifdef _MSC_VER
	char buf[256];
	bzero(buf, sizeof(buf));
	DWORD bytes = sizeof(buf) - 1;
	::GetComputerNameA(buf, &bytes);
	return buf;
#else
	ASSERT(FALSE);
	return "";
#endif
}

string ShellTool::GetUserName()
{
#ifdef _MSC_VER
	char buf[256];
	bzero(buf, sizeof(buf));
	DWORD bytes = sizeof(buf) - 1;
	::GetUserNameA(buf, &bytes);
	return buf;
#else
	ASSERT(FALSE);
	return "";
#endif
}

#ifdef _MSC_VER
#include <tlhelp32.h>

//http://www.neurophys.wisc.edu/ravi/software/killproc/
//2020.09.18,发现有准确，有时进程明明存在，但本接口搜索不到,但用CreateToolhelp32Snapshot可以获取到
int ShellTool::KillProcessByName(const char *szToTerminate)
// Created: 6/23/2000  (RK)
// Last modified: 3/10/2002  (RK)
// Please report any problems or bugs to kochhar@physiology.wisc.edu
// The latest version of this routine can be found at:
//     http://www.neurophys.wisc.edu/ravi/software/killproc/
// Terminate the process "szToTerminate" if it is currently running
// This works for Win/95/98/ME and also Win/NT/2000/XP
// The process name is case-insensitive, i.e. "notepad.exe" and "NOTEPAD.EXE"
// will both work (for szToTerminate)
// Return codes are as follows:
//   0   = Process was successfully terminated
//   603 = Process was not currently running
//   604 = No permission to terminate process
//   605 = Unable to load PSAPI.DLL
//   602 = Unable to terminate process for some other reason
//   606 = Unable to identify system type
//   607 = Unsupported OS
//   632 = Invalid process name
//   700 = Unable to get procedure address from PSAPI.DLL
//   701 = Unable to get process list, EnumProcesses failed
//   702 = Unable to load KERNEL32.DLL
//   703 = Unable to get procedure address from KERNEL32.DLL
//   704 = CreateToolhelp32Snapshot failed
// Change history:
//   modified 3/8/2002  - Borland-C compatible if BORLANDC is defined as
//                        suggested by Bob Christensen
//   modified 3/10/2002 - Removed memory leaks as suggested by
//					      Jonathan Richard-Brochu (handles to Proc and Snapshot
//                        were not getting closed properly in some cases)
{
	BOOL bResult;// , bResultm;
	DWORD aiPID[1000], iCb = 1000, iNumProc, iV2000 = 0;
	DWORD iCbneeded, i, iFound = 0;
	char szName[MAX_PATH], szToTermUpper[MAX_PATH];
	HANDLE hProc;// , hSnapShot;
	OSVERSIONINFO osvi = {0};
	HINSTANCE hInstLib=nullptr;
	int iLen, iLenP, indx;
	HMODULE hMod;
	//PROCESSENTRY32 procentry=nullptr;
	//MODULEENTRY32 modentry = nullptr;

	// Transfer Process name into "szToTermUpper" and
	// convert it to upper case
	iLenP = (int)strlen(szToTerminate);
	if (iLenP<1 || iLenP>MAX_PATH) return 632;
	for (indx = 0; indx<iLenP; indx++)
		szToTermUpper[indx] = toupper(szToTerminate[indx]);
	szToTermUpper[iLenP] = 0;

	// PSAPI Function Pointers.
	BOOL(WINAPI *lpfEnumProcesses)(DWORD *, DWORD cb, DWORD *);
	BOOL(WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *,
		DWORD, LPDWORD);
	DWORD(WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE,LPSTR, DWORD);

	// ToolHelp Function Pointers.
	//HANDLE(WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
	//BOOL(WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
	//BOOL(WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);
	//BOOL(WINAPI *lpfModule32First)(HANDLE, LPMODULEENTRY32);
	//BOOL(WINAPI *lpfModule32Next)(HANDLE, LPMODULEENTRY32);

	// First check what version of Windows we're in
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	//bResult = GetVersionEx(&osvi);
	//if (!bResult)     // Unable to identify system version
		//return 606;

	// At Present we only support Win/NT/2000/XP or Win/9x/ME
	//if ((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) &&
		//(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))
		//return 607;

	// Win/NT or 2000 or XP

	// Load library and get the procedures explicitly. We do
	// this so that we don't have to worry about modules using
	// this code failing to load under Windows 9x, because
	// it can't resolve references to the PSAPI.DLL.
	class AutoFree
	{
	public:
		AutoFree(HINSTANCE& module):mModule(module)
		{
		}

		~AutoFree()
		{
			if (mModule)
			{
				FreeLibrary(mModule);
				mModule = nullptr;
			}
		}

		HMODULE& mModule;
	};
	hInstLib = LoadLibraryA("PSAPI.DLL");
	AutoFree autoFree(hInstLib);
	if (hInstLib == NULL)
		return 605;

	// Get procedure addresses.
	lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *, DWORD, DWORD*))
		GetProcAddress(hInstLib, "EnumProcesses");
	lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
		DWORD, LPDWORD)) GetProcAddress(hInstLib,
			"EnumProcessModules");
	lpfGetModuleBaseName = (DWORD(WINAPI *)(HANDLE, HMODULE,LPSTR, DWORD)) GetProcAddress(hInstLib,"GetModuleBaseNameA");

	if (lpfEnumProcesses == NULL ||
		lpfEnumProcessModules == NULL ||
		lpfGetModuleBaseName == NULL)
	{
		
		return 700;
	}

	bResult = lpfEnumProcesses(aiPID, iCb, &iCbneeded);
	if (!bResult)
	{
		// Unable to get process list, EnumProcesses failed
		
		return 701;
	}

	// How many processes are there?
	iNumProc = iCbneeded / sizeof(DWORD);

	LogV(TAG, "process count=%d", iNumProc);
	// Get and match the name of each process
	map<DWORD, bool> pids;//sort pid for easy debug
	for (i = 0; i < iNumProc; i++)
	{
		// Get the (module) name for this process

		strcpy(szName, "Unknown");
		const auto pid = aiPID[i];
		pids[pid] = true;
	}

	for (auto& item:pids)
	{
		strcpy(szName, "Unknown");
		const auto pid = item.first;
		hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		// Now, get the process name
		if (hProc)
		{
			if (lpfEnumProcessModules(hProc, &hMod, sizeof(hMod), &iCbneeded))
			{
				iLen = lpfGetModuleBaseName(hProc, hMod, szName, MAX_PATH);
				//LogI(TAG, "%s,pid=%d", szName,pid);
			}
			CloseHandle(hProc);
		}
		else
		{
			//LogV(TAG,"fail open process %d(0x%x)",pid,pid);
		}

		if (strcmp(_strupr(szName), szToTermUpper) == 0)
		{
			// Process found, now terminate it
			iFound = 1;
			
			hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
			if (hProc)
			{
				if (TerminateProcess(hProc, 0))
				{
					// process terminated
					CloseHandle(hProc);
					return 0;
				}
				else
				{
					// Unable to terminate process
					CloseHandle(hProc);
					
					return 602;
				}
			}
			else
			{
				LogW(TAG, "fail OpenProcess(%d),error=%d,app=[%s]", pid, GetLastError(), szToTerminate);
				// Unable to open process for termination
				
				return 604;
			}
		}
	}

	if (iFound == 0)
	{
		
		return 603;
	}

	
	return 0;
}

BOOL ShellTool::CopyTextToClipboardGB2312(HWND hWnd, const std::string& text)
{
	USES_CONVERSION;
	CString t = A2T(text.c_str());
	return CopyTextToClipboard(hWnd, t);
}

BOOL ShellTool::CopyTextToClipboard(HWND hWnd, const std::string& text)
{
	auto t=Utf8Tool::UTF8_to_UNICODE(text.c_str(), text.length());
	return CopyTextToClipboard(hWnd, t);
}

BOOL ShellTool::CopyTextToClipboard(HWND hWnd, CString text)
{
	if (!::OpenClipboard(hWnd))
	{
		return FALSE;
	}

	EmptyClipboard();

	int cch = text.GetLength();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(TCHAR));
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return FALSE;
	}

	// Lock the handle and copy the text to the buffer. 

	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, text, cch * sizeof(TCHAR));
	lptstrCopy[cch] = (TCHAR)0;    // null character 
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard. 
	auto uFormat = CF_UNICODETEXT;
#ifndef _UNICODE
	uFormat = CF_TEXT;
#endif
	SetClipboardData(uFormat, hglbCopy);
	CloseClipboard();

	return TRUE;
}

int ShellTool::ShowInFolder(const string& filePath)
{
	int ret = -1;

	if (!filePath.empty() && File::FileExists(filePath.c_str()))
	{
		string cmd;
		cmd= StringTool::Format("\"%s\"", filePath.c_str());
		StringTool::Replace(cmd,"/", "\\");
		StringTool::Replace(cmd,"\\\\", "\\");

		ShellExecuteA(NULL, "open", "Explorer.exe", (string(" /select,") + cmd).c_str(), NULL, SW_SHOWDEFAULT);
		ret = 0;
	}

	return ret;
}

#endif

//text1,text2,....
void ShellTool::String2Vector(const string& text, vector<string>& vec)
{
	vec.clear();

	TextSeparator demux(text.c_str(), ",");
	string item;
	while (1)
	{
		int ret = demux.GetNext(item);
		if (ret)
		{
			break;
		}

		if (item.empty())
		{
			continue;
		}

		if (IsExists(vec, item.c_str()))
		{
			continue;
		}

		vec.push_back(item.c_str());
	}
}

//把number1,number2,....的字符串的数字解析出来，并保证不重复
void ShellTool::String2IntVector(const string& text, vector<int>& vec)
{
	vec.clear();

	TextSeparator demux(text.c_str(), ",");
	string item;
	while (1)
	{
		int ret = demux.GetNext(item);
		if (ret)
		{
			break;
		}

		if (item.empty())
		{
			continue;
		}

		int id = atoi(item.c_str());
		if (IsExists(vec, id))
		{
			continue;
		}

		vec.push_back(id);
	}
}

bool ShellTool::IsExists(vector<string>& vec, string value)
{
	int nc = (int)vec.size();
	for (int i = 0; i < nc; i++)
	{
		if (vec[i] == value)
		{
			return true;
		}
	}

	return false;
}

bool ShellTool::IsExists(vector<int>& vec, int number)
{
	int nc = (int)vec.size();
	for (int i = 0; i < nc; i++)
	{
		if (vec[i] == number)
		{
			return true;
		}
	}

	return false;
}

void ShellTool::DumpCallStack()
{
	DumpCallStackX();
}

#ifndef _MSC_VER
//注意:需要调用free()来释放返回的数据
char **ShellTool::CommandLineToArgvW(const char *cmdline, int& nc)
{
	//实现类似windows CommandLineToArgvW的功能

	nc = 0;

	if (!cmdline)
	{
		return NULL;
	}

	char *ackBuf = NULL;
	vector<string> arr;
	ParseCommandLine(cmdline, arr);	
	nc = (int)arr.size();
	if (nc>0)
	{
		int totalLen = 0;
		totalLen += (int)(nc * sizeof(char*));//pointer len

		int i = 0;
		for (i = 0; i<nc; i++)
		{
			string item = arr[i];
			totalLen += (int)item.length() + 1;
		}

		totalLen++;//an extra tail '\0'

		ASSERT(!ackBuf);
		ackBuf = (char*)malloc(totalLen);
		memset(ackBuf, 0, totalLen);

		long *ptr = (long*)ackBuf;
		char *psz = ackBuf + nc * sizeof(char*);

		for (i = 0; i<nc; i++)
		{
			string item= arr[i];
			int len = (int)item.length() + 1;//copy '\0'
			memcpy(psz, item.c_str(), len);

			*ptr++ = (long)psz;	//save pointer
			psz += len;
		}
	}

	return (char**)ackBuf;
}
#endif

/*
[2013.11.20 16:00:49]::[WinExec[iwpriv ra0 set SSID="may2013.6"]](/mnt/work/board/cs_cyc_svn2787/core/base/shelltool.cpp:97)
[2013.11.20 16:00:49]::[prefork,cmd=[iwpriv ra0 set SSID="may2013.6"]](/mnt/work/board/cs_cyc_svn2787/core/base/systemcommand.cpp:120)
[2013.11.20 16:00:49]::[args[0]=[iwpriv]](/mnt/work/board/cs_cyc_svn2787/core/base/systemcommand.cpp:123)
[2013.11.20 16:00:49]::[args[1]=[ra0]](/mnt/work/board/cs_cyc_svn2787/core/base/systemcommand.cpp:123)
[2013.11.20 16:00:49]::[args[2]=[set]](/mnt/work/board/cs_cyc_svn2787/core/base/systemcommand.cpp:123)
[2013.11.20 16:00:49]::[args[3]=[SSID=may2013.6]](/mnt/work/board/cs_cyc_svn2787/core/base/systemcommand.cpp:123)
注意解析后的args[3]中不能带",否则iwpriv连接不上wifi
//*/
void ShellTool::ParseCommandLine(const char *cmdline, vector<string>& args)
{
	args.clear();

	if (!cmdline)
	{
		return;
	}

	enum eStatus
	{
		eStatus_Normal,
		eStatus_SQ,		//single quotes
		eStatus_DQ,		//double quotes
	}status = eStatus_Normal;

	string header;
	const char *ps = cmdline;
	int len = (int)strlen(cmdline);
	for (int i = 0; i<len; i++)
	{
		char ch = cmdline[i];
		bool argEnd = false;
		switch (status)
		{
		case eStatus_Normal:
		{
			if (ch == ' ')
			{
				argEnd = true;
			}
			else if (ch == '\'')
			{
				status = eStatus_SQ;
				header = string(ps, (int)(cmdline + i + 1 - ps));
				ps = cmdline + i + 1;
			}
			else if (ch == '\"')
			{
				status = eStatus_DQ;

				header = string(ps, (int)(cmdline + i + 1 - ps - 1));
				ps = cmdline + i + 1;
			}
			break;
		}
		case eStatus_SQ:
		{
			if (ch == '\'')
			{
				argEnd = true;
			}

			break;
		}
		case eStatus_DQ:
		{
			if (ch == '\"')
			{
				argEnd = true;
			}

			break;
		}
		default:
		{
			ASSERT(FALSE);
			break;
		}
		}

		if (i == len - 1)
		{
			i = len;
			argEnd = true;
		}

		if (argEnd)
		{
			string arg(ps, (int)(cmdline + i - ps));
			if (!arg.empty())
			{
				string lastChar = arg.substr(arg.length()-1);
				if (lastChar == "\'" || lastChar == "\"")
				{
					arg = arg.substr(0,arg.length() - 1);
				}

				string item = header + arg;
				//DT("[%s]",item.c_str());
				args.push_back(item.c_str());
			}

			header.clear();

			ps = cmdline + i + 1;

			status = eStatus_Normal;
		}
	}
}


#ifdef _SUPPORT_SSL
#include "crypt/ssl/sha1.h"
void ShellTool::sha1_EX(unsigned char *input, int ilen, unsigned char output[20])
{
	::sha1_EX(input, ilen, output);
}
#endif

#ifdef _MSC_VER
int ShellTool::System(const char *szCmd)
{
	return ::system(szCmd);
}

#else
#include <sys/types.h>
#include <sys/wait.h>
#ifndef SUCCESS
#define SUCCESS 0
#define FAILURE -1
#endif

//不要使用linux api system(),它有很多坑，经常不按期望的工作，手工在secureCRT中运行正常，用system()来调用就不正常
int ShellTool::System(const char *szCmd)
{
	LogV(TAG,"System(%s)", szCmd);

	pid_t pid = 0;
	int ret = SUCCESS;
	int nRetry = 5;

	if (NULL == szCmd)
	{
		return FAILURE;
	}

	while (0 < nRetry--)
	{
		if (0 > (pid = vfork()))
		{
			sleep(1);
		}
		else
		{
			break;
		}
	}

	if (0 > pid)
	{
		LogW(TAG,"fork failed retry=%d\n", nRetry);
		ret = FAILURE;
	}
	else if (0 == pid)
	{
		int fd;
#ifdef _CONFIG_ANDROID
		int maxfd=sysconf(_SC_OPEN_MAX);
		for (fd = 3; fd < maxfd; fd++)
			close(fd);
#else
		for (fd = 3; fd < getdtablesize(); fd++)
			close(fd);
#endif

		execl("/bin/sh", "sh", "-c", szCmd, NULL);
		exit(127);
	}
	else
	{
		while (0 > waitpid(pid, &ret, 0))
		{
			if (EINTR != errno)
			{
				ret = FAILURE;
				break;
			}
		}
	}

	return ret;
}

#endif

static ULONG gGmtSecondOffset = 8 * 3600;//时区偏移秒数,默认为中国时区GMT+08:00
static const char mon_list[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const char leap_mon_list[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

time_t DateTime::mktime(tm* pT)
{
	const char* pDays = NULL;
	time_t tmp = 0;
	int16_t i = 0;

	pT->tm_year += 1900;
	pT->tm_mon += 1;

	//计算总共有多少个闰年
	tmp = (pT->tm_year / 4 - pT->tm_year / 100 + pT->tm_year / 400) - (1970 / 4 - 1970 / 100 + 1970 / 400);

	//如果当年是闰年，需要减去当年的闰年
	if ((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0)))
	{
		tmp = tmp - 1 + (pT->tm_year - 1970) * 365;
		pDays = leap_mon_list;
	}
	else
	{
		tmp = tmp + (pT->tm_year - 1970) * 365;
		pDays = mon_list;
	}

	for (i = 0; i < pT->tm_mon - 1; i++)
		tmp += pDays[i];

	tmp = tmp + pT->tm_mday - 1;

	tmp = tmp * 24 + pT->tm_hour;

	tmp = tmp * 60 + pT->tm_min;

	tmp = tmp * 60 + pT->tm_sec;
	tmp -= gGmtSecondOffset;//减去默认加上的GMT+08:00

	return tmp;
}

/*
时间戳转为时间
tim:当前时间戳
tm *pT： 输出的时间缓冲区
*/
void DateTime::localtime(time_t tim, tm* pT)
{
	const char* pDays = NULL;

	uint16_t index = 0;

	//memset(pT, 0, sizeof(*pT));
	tim += gGmtSecondOffset;

#if 1
  //year initialization
	if (tim >= 0x5685C180L)            // 2016-1-1 0:0:0
	{
		pT->tm_year = 2016;
		tim -= 0x5685C180L;
	}
	else if (tim >= 0x4B3D3B00L)       // 2010-1-1 0:0:0
	{
		pT->tm_year = 2010;
		tim -= 0x4B3D3B00L;
	}
	else if (tim >= 0x386D4380L)       // 2000-1-1 0:0:0
	{
		pT->tm_year = 2000;
		tim -= 0x386D4380L;
	}
	else
	{
		pT->tm_year = 1970;
	}

	//now have year
	while (tim >= 366L * 24 * 60 * 60)
	{
		if ((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0)))
			tim -= 366L * 24 * 60 * 60;
		else
			tim -= 365L * 24 * 60 * 60;

		pT->tm_year++;
	}

	// then 365 * 24 * 60 * 60 < tim < 366 * 24 * 60 * 60
	if (!(((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0))))
		&& (tim >= 365L * 24 * 60 * 60))
	{
		tim -= 365L * 24 * 60 * 60;
		pT->tm_year++;
	}

	// this year is a leap year?
	if (((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0))))
		pDays = leap_mon_list;
	else
		pDays = mon_list;

	pT->tm_mon = 0;
	// now have mon
	while (tim >= pDays[index] * 24L * 60 * 60)
	{
		tim -= pDays[index] * 24L * 60 * 60;
		index++;
		pT->tm_mon++;
	}

	// now have days
	pT->tm_mday = (int)(tim / (24L * 60 * 60) + 1);
	tim = tim % (24L * 60 * 60);

	// now have hour
	pT->tm_hour = (int)(tim / (60 * 60));
	tim = tim % (60 * 60);

	// now have min
	pT->tm_min = (int)(tim / 60);
	tim = tim % 60;

	pT->tm_sec = (int)(tim);

	pT->tm_year -= 1900;//符合linux/windows api标准
#endif
}

//返回tm1晚于tm2的天数
//比如tm1是tm2后一天，则返回1
//如果tm2晚于tm1,返回天数为负数
int DateTime::spanDays(const tm& tm1, const tm& tm2)
{
	tm tt1 = tm1;
	tm tt2 = tm2;
	time_t t1 = DateTime::mktime(&tt1);
	time_t t2 = DateTime::mktime(&tt2);

	int days = (int)((t1 - t2) / (3600 * 24));
	return days;
}

static time_t gBaseTime = 0;//可由外界设定

time_t DateTime::time()
{
#ifdef _MSC_VER
	return ::time(nullptr);
#else
	return (ShellTool::GetTickCount() / 1000) + gBaseTime;
#endif
}

//timeStr可为yyyymmddhhMMss，yyyy-mm-dd hh:MM:ss,yyyy.mm.dd hh:MM:ss
int tagTimeMs::setTime(const string& timeStr, const string& desc)
{
	if (timeStr.empty())
	{
		return -1;
	}

#if defined _MSC_VER || defined _ANDROID || defined _CONFIG_PC_LINUX
	LogV(TAG, "skip %s",__func__);
	return 0;
#else
	tagTimeMs time;
	if (String2TimeMs(timeStr, time))
	{
		return -1;
	}

	auto t = time.to_time_t();
	int ret = stime(&t);
	if (ret == 0)
	{
		auto tWant=time.to_time_t();
		auto tNow = tagTimeMs::now().to_time_t();

		auto delta = (int)(tWant - tNow);
		LogV(TAG, "%s delta=%d", __func__, delta);

		if (tWant > tNow + 3600 * 7 && tWant < tNow + 3600 * 9)
		{
			LogV(TAG, "auto fix timezone#begin");
			t += 3600 * 8;
			ret=stime(&t);
			LogV(TAG, "auto fix timezone#end");
		}
	}

	LogV(TAG, "stime ret=%d", ret);

#endif
}

tagTimeMs tagTimeMs::now()
{
#ifdef _MSC_VER
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int year = st.wYear;
	int month = st.wMonth;
	int day = st.wDay;
	int hour = st.wHour;
	int minute = st.wMinute;
	int second = st.wSecond;
	int ms = st.wMilliseconds;
#else
	struct timeval tv = { 0 };
	gettimeofday(&tv, nullptr);
	time_t time = tv.tv_sec;
	auto ms = (int)(tv.tv_usec / 1000);

	struct tm tmNow;
	localtime_r(&time, &tmNow);
	int year = tmNow.tm_year + 1900;
	int month = tmNow.tm_mon + 1;
	int day = tmNow.tm_mday;
	int hour = tmNow.tm_hour;
	int minute = tmNow.tm_min;
	int second = tmNow.tm_sec;
#endif

	tagTimeMs obj;
	obj.year = year;
	obj.month = month;
	obj.day = day;
	obj.hour = hour;
	obj.minute = minute;
	obj.second = second;
	obj.ms = ms;
	return obj;
}

time_t tagTimeMs::to_time_t()const
{
	tm tm1 = { 0 };
	tm1.tm_year = year - 1900;
	tm1.tm_mon = month - 1;
	tm1.tm_mday = day;
	tm1.tm_hour = hour;
	tm1.tm_min = minute;
	tm1.tm_sec = second;
	time_t t1 = DateTime::mktime(&tm1);
	return t1;
}

//返回晚于obj的天数
//比如是obj的后一天，则返回1
//如果obj晚于本对象,返回天数为负数
int tagTimeMs::laterDays(const tagTimeMs& obj)
{
	tm tm1 = { 0 };
	tm1.tm_year = this->year - 1900;
	tm1.tm_mon = this->month - 1;
	tm1.tm_mday = this->day;
	//time_t t1 = DateTime::mktime(&tm1);

	tm tm2 = { 0 };
	tm2.tm_year = obj.year - 1900;
	tm2.tm_mon = obj.month - 1;
	tm2.tm_mday = obj.day;

	//time_t t2 = DateTime::mktime(&tm2);
	int days = DateTime::spanDays(tm1, tm2);
	return days;
}

//支持的几种dt样本:
// 20220716123456789,其中789是ms
// 20220716123456
// 20220716
void tagTimeMs::from_dt(const string& dt)
{
	clear();
	if (dt.length() >= strlen("20220716"))
	{
		int date = atoi(dt.substr(0,8).c_str());
		int time = atoi(dt.substr(8,6).c_str());
		int ms_ = atoi(dt.substr(14, 3).c_str());

		from_dt(date, time);
		ms = ms_;
	}

	if (year == 0)
	{
		String2TimeMs(dt, *this);
	}
}

void tagTimeMs::from_dt(int date, int time)
{
	year = date / 10000;
	month = (date / 100) % 100;
	day = date % 100;
	hour = time / 10000;
	minute = (time/100)%100;
	second = time % 100;
	ms = 0;
}

tagTimeMs& tagTimeMs::from_time_t(time_t t)
{
	tm time = { 0 };
	DateTime::localtime(t, &time);

	year = time.tm_year + 1900;
	month = time.tm_mon + 1;
	day = time.tm_mday;
	hour = time.tm_hour;
	minute = time.tm_min;
	second = time.tm_sec;
	ms = 0;
	return *this;
}

/* 对接第三方平台时经常用到，标准日期时间字符串 */
string tagTimeMs::stdDateTimeText()const
{
	string text = StringTool::Format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	return text;
}

string tagTimeMs::stdDateText()const
{
	string text = StringTool::Format("%04d-%02d-%02d");
	return text;
}

string tagTimeMs::stdTimeText()const
{
	string text = StringTool::Format("%02d:%02d:%02d", hour, minute, second);
	return text;
}

string tagTimeMs::toDT()const
{
	string text = StringTool::Format("%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);
	return text;
}

string tagTimeMs::toText()const
{
	string text = StringTool::Format("%04d.%02d.%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	return text;
}

void tagTimeMs::daySeconds2Time(int seconds)
{
	seconds %= (24 * 3600);

	hour = seconds / 3600;
	seconds -= hour * 3600;

	minute = seconds / 60;
	seconds -= minute * 60;

	second = seconds;
}

int tagTimeMs::String2TimeMs(const string& text, tagTimeMs& ms)
{
	tagTimeMs& t = ms;
	int ret = sscanf(text.c_str(), "%04d.%02d.%02d %02d:%02d:%02d"
		, &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second
	);

	if (ret == 6)
	{
		return 0;
	}

	ret = sscanf(text.c_str(), "%04d-%02d-%02d %02d:%02d:%02d"
		, &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second
	);

	if (ret == 6)
	{
		return 0;
	}

	if (text.length() == strlen("20220416160000"))
	{
		int off = 0;
		t.year = atoi(text.substr(off,4).c_str());
		off += 4;

		t.month = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.day = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.hour = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.minute = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.second = atoi(text.substr(off, 2).c_str());
		off += 2;

		return 0;
	}

	LogV(TAG, "fail %s,text=%s", __func__, text.c_str());
	return -1;
}

/*
addr支持三种格式
addr=十进制地址
addr=0x/0Xabcd,十六进制地址
addr=xxKB/kb,以整数KB为单位
*/
uint32_t ShellTool::str2Int(const string& addr)
{
	uint32_t v = 0;

	if (addr.find("KB") != string::npos || addr.find("kb") != string::npos) {
		v = 1024 * (uint32_t)atoll(addr.c_str());
	}
	else if (addr.find("0x") == 0 || addr.find("0X") == 0) {
		v = strtoll(addr.c_str() + 2, nullptr, 16);
	}
	else {
		
		v = (uint32_t)atoll(addr.c_str());
	}

	return v;
}


