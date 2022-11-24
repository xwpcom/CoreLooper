#pragma once
//#include "stringex.h"
namespace Bear {
namespace Core
{
namespace FileSystem
{
	class IniFile;
};
using namespace FileSystem;
using namespace std;
#ifdef _MSC_VER
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#else
typedef void* (*LPTHREAD_START_ROUTINE)(LPVOID lpParameter);
#include <time.h>
#include <sys/times.h>
#include "core/base/win32.h"
typedef void* (*LPTHREAD_START_ROUTINE)(LPVOID lpParameter);
typedef struct GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	BYTE           Data4[8];
}__attribute__((packed)) GUID;

#endif

#define CLAMP(minValue,value,maxValue) do {if(value < minValue) value=minValue;if(value>maxValue) value=maxValue; }while(0)

//从stcapp c51工程引入
// XiongWanPing 2021.12.31
//来源 https://www.cnblogs.com/hiker-blogs/p/C51.html  
//注意: 这个网文的代码有bug,有几处>要改为>=,否则会出现2021-12-32 00:00:00类似的非法日期

class CORE_EXPORT DateTime
{
public:
	static time_t time();
	static time_t mktime(tm* pT);
	static void localtime(time_t tim, tm* pT);
	static int spanDays(const tm& tm1, const tm& tm2);
};

//time accurate to ms
struct CORE_EXPORT tagTimeMs
{
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int ms = 0;

	tagTimeMs()
	{
		clear();
	}
	
	void clear()
	{
		memset(this, 0, sizeof(*this));
	}

	tagTimeMs(time_t t)
	{
		from_time_t(t);
	}

	int date()const
	{
		return year * 10000 + month * 100 + day;
	}

	int time()const
	{
		return hour * 10000 + minute * 100 + second;
	}

	time_t to_time_t()const;
	void from_time_t(time_t t);
	void from_dt(int date,int time);
	void from_dt(const string& dt);

	int laterDays(const tagTimeMs& obj);
	int currentDaySeconds()
	{
		return hour * 3600 + minute * 60 + second;
	}
	static int time2Seconds(int time)
	{
		//time格式:hhmmss

		int h = time / 10000;
		int m = (time / 100) % 100;
		int s = time % 100;
		return h * 3600 + m * 60 + s;
	}

	static int String2TimeMs(const string& text, tagTimeMs& ms);
	string toText()const;
	string stdDateTimeText()const;
	static tagTimeMs now();
};

class CORE_EXPORT ShellTool
{
public:

	static void String2IntVector(const string& text, vector<int>& vec);
	static void String2Vector(const string& text, vector<string>& vec);
	static bool IsExists(vector<int>& vec, int number);
	static bool IsExists(vector<string>& vec, string value);


	static string GetComputerName();
	static string GetUserName();

	static DWORD GetTickCount();
	static ULONGLONG GetTickCount64();
	static int GetLastError();
	static void Sleep(UINT ms);
	static struct tagTimeMs GetCurrentTimeMs();

#ifdef _MSC_VER	
	static struct tagTimeMs GetRelativeTimeMs(int deltaDays);
	static string CreateGuid();
	static string CreateGuidToken();
#endif

	static DWORD GetCurrentProcessId();
	static DWORD GetCurrentThreadId();
	static void SetThreadName(LPCSTR szThreadName, DWORD dwThreadID = GetCurrentThreadId());

	static BOOL QueueUserWorkItem(
		LPTHREAD_START_ROUTINE Function,  // starting address
		PVOID Context,                    // function data
		ULONG Flags = 0                       // worker options
	);
	static void DumpCallStack();
	static string GetAppPathName();
	static string GetAppPath();
	static string GetAppDisk();
	static GUID UuidFromString(string szGuid);
	static string GuidToString(GUID& guid);

	static struct tm GetLocalTime(int deltaSecond = 0);
	static string Second2Str(int seconds);
	static string Bytes2String(LPBYTE pBuf, int cbBuf);
	static int PrintHex(LPBYTE pBuf, int cbBuf, const char *desc);

#ifdef _MSC_VER
	static string RelativePath2FullPath(string relativePath);
#endif

	static void ParseCommandLine(const char *cmdline, vector<string>& args);

#ifdef _MSC_VER
	static BOOL CopyTextToClipboard(HWND hWnd, const string& text);
	static BOOL CopyTextToClipboard(HWND hWnd, CString text);
	static BOOL CopyTextToClipboardGB2312(HWND hWnd, const string& text);
	static int ShowInFolder(const string& filePath);
	static int KillProcessByName(const char *szToTerminate);
	static LPWSTR * CommandLineToArgvW(LPCWSTR lpCmdLine, int *pNumArgs);
	static void SaveWindowPos(HWND hWnd, CString szWinName);
	static void LoadWindowPos(HWND hWnd, CString szWinName, BOOL bShow = TRUE);
	static void SaveWindowPosHelper(HWND hWnd, CString szWinName, BOOL bSave, BOOL bShow = TRUE);

	static void SaveWindowPos(HWND hWnd, CString szWinName, IniFile& ini);
	static void LoadWindowPos(HWND hWnd, CString szWinName,IniFile& ini);
	static void SaveWindowPosHelper(HWND hWnd, CString szWinName, BOOL bSave, IniFile& ini);

	static void EatUpMessage(BOOL bDiscardUserOp = TRUE);
	static void DispatchMessageWaitFalse(BOOL& bNeedWait, HWND hWndTempDisable = NULL);
	static BOOL WaitThreadExitEx(HANDLE handle);
#else
	//注意:需要调用free()来释放返回的数据
	static char **CommandLineToArgvW(const char *cmdline, int& nc);
#endif
#ifdef __AFX_H__
	static void OnInitMenuPopupHelper(CWnd *pWnd, CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu);
#endif
#ifdef _SUPPORT_SSL
	static void sha1_EX(unsigned char *input, int ilen, unsigned char output[20]);
#endif

	static int System(const char *szCmd);
	static int System(const string& cmd)
	{
		return System(cmd.c_str());
	}
};

}
}
