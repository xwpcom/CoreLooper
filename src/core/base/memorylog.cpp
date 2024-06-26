﻿#include "stdafx.h"
#include "memorylog.h"

#ifdef _MSC_VER_DEBUG
//#define new DEBUG_NEW //不兼容c++ placement new,所以屏蔽它

namespace Bear {
namespace Core {

const auto* TAG = "MemoryLog";

BYTE gMemoryLogBuf[sizeof(MemoryLog)];
MemoryLog* gMemoryLog;
class MemoryLogFactory
{
public:
	MemoryLogFactory()
	{
		//在gMemoryLogBuf上主动调用MemoryLog构造函数
		void* p = (MemoryLog*)gMemoryLogBuf;
		gMemoryLog = new(p) MemoryLog();
	}
};
MemoryLogFactory gMemoryLogFactory;//确保单线程中初始化，避免竞争


void DisplayError(TCHAR* pszAPI, DWORD dwError)
{
    LPWSTR lpvMessageBuffer;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpvMessageBuffer, 0, NULL);

    //... now display this string
    _tprintf(TEXT("ERROR: API        = %s\n"), pszAPI);
    _tprintf(TEXT("       error code = %d\n"), dwError);
    _tprintf(TEXT("       message    = %s\n"), lpvMessageBuffer);

    // Free the buffer allocated by the system
    LocalFree((LPVOID*)lpvMessageBuffer);

    ExitProcess(GetLastError());
}

void Privilege(TCHAR* pszPrivilege, BOOL bEnable)
{
    HANDLE           hToken;
    TOKEN_PRIVILEGES tp;
    BOOL             status;
    DWORD            error;

    // open process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        DisplayError(TEXT("OpenProcessToken"), GetLastError());

    // get the luid
    if (!LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid))
        DisplayError(TEXT("LookupPrivilegeValue"), GetLastError());

    tp.PrivilegeCount = 1;

    // enable or disable privilege
    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // enable or disable privilege
    status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    // It is possible for AdjustTokenPrivileges to return TRUE and still not succeed.
    // So always check for the last error value.
    error = GetLastError();
    if (!status || (error != ERROR_SUCCESS))
        DisplayError(TEXT("AdjustTokenPrivileges"), GetLastError());

    // close the handle
    if (!CloseHandle(hToken))
        DisplayError(TEXT("CloseHandle"), GetLastError());
}


MemoryLog::MemoryLog()
{

    /*
    https://github.com/mutouyun/cpp-ipc/wiki/ipc::spin_lock
ipc::spin_lock
木头云 edited this page on Apr 3, 2019 · 2 revisions
简单的自旋锁实现。其实例可以放入共享内存中作为进程间的互斥锁。
    
    https://github.com/LiveAsynchronousVisualizedArchitecture/simdb
    A high performance, shared memory, lock free, cross platform, single file, no dependencies, C++11 key-value store

    */


    //Privilege(TEXT("SeLockMemoryPrivilege"), TRUE);
	//LogV(TAG,"%s,this=%p", __func__, this);

#define BUF_SIZE 256
    TCHAR szName[] = TEXT("CoreLooperMemoryLog");
    TCHAR szMsg[] = TEXT("Message from first process.");

    auto hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        BUF_SIZE,                // maximum object size (low-order DWORD)
        szName);                 // name of mapping object

    if (hMapFile)
    {
        auto p = (LPBYTE)MapViewOfFile(hMapFile,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0,
            0,
            BUF_SIZE);

        if (p)
        {
            p[1] = 0x01;
            p[2] = 0x02;
            p[3] = 0x03;
        }
    }

}

MemoryLog::~MemoryLog()
{
	LogV(TAG, "%s,this=%p", __func__, this);
}

void MemoryLog::AddLog(const char* msg)
{
}

}
}

#endif
