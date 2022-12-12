#include "stdafx.h"
#include "dbghelper.h"
#include <tlhelp32.h>
#include <dbghelp.h>
#pragma comment ( lib, "dbghelp.lib" )

namespace Windows {
/*
https://learn.microsoft.com/en-us/windows/win32/debug/minidump-files
https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
https://learn.microsoft.com/en-us/windows/win32/toolhelp/process-walking
*/

class AutoClose
{
public:
	AutoClose(HANDLE& handle) :mHandle(handle)
	{
	}
	~AutoClose()
	{
		if (mHandle)
		{
			CloseHandle(mHandle);
			mHandle = nullptr;
		}
	}
protected:
	HANDLE& mHandle;
};

DbgHelper::DbgHelper()
{

}

/*
返回第一个匹配exeName的进程id
exeName样本:IotPlatform.exe,不区分大小写
返回0表示没有找到
*/
int DbgHelper::processId(const string& exeName)
{
	auto hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	AutoClose autoClose(hProcessSnap);

	//LogV(mTag, "CreateToolhelp32Snapshot = %p", hProcessSnap);

	if (hProcessSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

		// Retrieve information about the first process, and exit if unsuccessful
		if (!Process32First(hProcessSnap, &pe32))
		{
			LogW(mTag, "fail Process32First"); // show cause of failure
			return 0;
		}

		USES_CONVERSION;
		int index = -1;
		do
		{
			++index;

			const auto pid = pe32.th32ProcessID;
			string app = T2A(pe32.szExeFile);
			//LogI(mTag, "process[%04d]=[%s]", index, app.c_str());

			if (StringTool::CompareNoCase(app, exeName) == 0)
			{
				return pid;
			}
		} while (Process32Next(hProcessSnap, &pe32));
	}

	return 0;
}

int DbgHelper::test()
{
	string exeName = "dt.exe";
	createMiniDump(exeName, "d:/mini.dmp");
	return 0;

	auto pid = processId(exeName);
	LogV(mTag, "%s pid=%d", exeName.c_str(), pid);

	auto count = GetActiveProcessorCount(
		ALL_PROCESSOR_GROUPS
	);

	LogV(mTag, "cpu count=%d", count);
	return 0;
}

int DbgHelper::createMiniDump(const string& exeName, const string& dumpFilePath)
{
	auto pid = processId(exeName);
	if (pid == 0)
	{
		LogW(mTag, "no found process[%s]", exeName.c_str());
		return -1;
	}

	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	File::CreateFolderForFile(dumpFilePath);
	HANDLE hFile = CreateFileA(dumpFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_TYPE mdt = static_cast<MINIDUMP_TYPE>(MiniDumpNormal 
		| MiniDumpWithDataSegs
		| MiniDumpWithHandleData 
		| MiniDumpWithThreadInfo
		//| MiniDumpFilterModulePaths
		//| MiniDumpWithProcessThreadData
		//| MiniDumpWithCodeSegs
		//| MiniDumpWithTokenInformation
		//| MiniDumpWithModuleHeaders

		);
	BOOL ok = MiniDumpWriteDump(hProcess, pid, hFile, mdt, 0, 0, 0);

	CloseHandle(hFile);
	CloseHandle(hProcess);

	LogV(mTag, "%s minidump ret = %s", exeName.c_str(), ok?"success":"fail");
	return ok?0:-1;
}

}
