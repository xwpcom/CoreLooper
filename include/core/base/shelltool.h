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

//time accurate to ms
struct tagTimeMs
{
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int ms = 0;

	int date()const
	{
		return year * 10000 + month * 100 + day;
	}

	int time()const
	{
		return hour * 10000 + minute * 100 + second;
	}
};

class CORE_EXPORT ShellTool
{
public:

	static void String2IntVector(const std::string& text, std::vector<int>& vec);
	static void String2Vector(const std::string& text, std::vector<std::string>& vec);
	static bool IsExists(std::vector<int>& vec, int number);
	static bool IsExists(std::vector<std::string>& vec, std::string value);


	static std::string GetComputerName();
	static std::string GetUserName();

	static DWORD GetTickCount();
	static ULONGLONG GetTickCount64();
	static int GetLastError();
	static void Sleep(UINT ms);
	static struct tagTimeMs GetCurrentTimeMs();

#ifdef _MSC_VER	
	static struct tagTimeMs GetRelativeTimeMs(int deltaDays);
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
	static std::string GetAppPathName();
	static std::string GetAppPath();
	static std::string GetAppDisk();
	static GUID UuidFromString(std::string szGuid);
	static std::string GuidToString(GUID& guid);

	static struct tm GetLocalTime(int deltaSecond = 0);
	static std::string Second2Str(int seconds);
	static std::string Bytes2String(LPBYTE pBuf, int cbBuf);
	static int PrintHex(LPBYTE pBuf, int cbBuf, const char *desc);

#ifdef _MSC_VER
	static std::string RelativePath2FullPath(std::string relativePath);
	static bool IsDeveloperPC();
	static bool IsHongKongServer();
	static bool IsChinaServer();
#endif

	static void ParseCommandLine(const char *cmdline, std::vector<std::string>& args);

#ifdef _MSC_VER
	static BOOL CopyTextToClipboard(HWND hWnd, const std::string& text);
	static BOOL CopyTextToClipboard(HWND hWnd, CString text);
	static BOOL CopyTextToClipboardGB2312(HWND hWnd, const std::string& text);
	static int ShowInFolder(const std::string& filePath);
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
};

}
}
