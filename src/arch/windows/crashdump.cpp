#include "stdafx.h"
#include "arch/windows/crashdump.h"
#include <string>
using namespace std;
#include "shelltool.h"
using namespace Bear::Core;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment ( lib, "dbghelp.lib" )

char CCrashDump::m_szPrefix[64];
BOOL CCrashDump::m_bInstalled=FALSE;

/*
2021.01.03
用如下代码测试发现,stack overflow时会在StartAutoDump中报错导致没法生成dmp文件

	class FillStack
	{
	public:
		void func()
		{
			char buf[4096];
			buf[0] = 0;
			FillStack obj;
			obj.func();
		}
	};

	FillStack obj;
	obj.func();

*/

//已测试确认，release版不加debug信息，生成的.dmp文件也能定位到bug代码行，前提是生成.exe后代码没修改过
long CCrashDump::StartAutoDump(struct _EXCEPTION_POINTERS *pep)
{
	char	szFileName[_MAX_PATH]	= {0};
	SYSTEMTIME stLocalTime;
	GetLocalTime( &stLocalTime );
	string folder = ShellTool::GetAppPath();
	if(m_szPrefix[0])
	{
		sprintf( szFileName,"%s/%s_%d%02d%02d_%02d%02d%02d.dmp",
			folder.c_str(),
			m_szPrefix,
			stLocalTime.wYear,stLocalTime.wMonth,stLocalTime.wDay,stLocalTime.wHour,stLocalTime.wMinute,stLocalTime.wSecond);  
	}
	else
	{
		sprintf( szFileName,"%s/%d%02d%02d_%02d%02d%02d.dmp",
			folder.c_str(),
			stLocalTime.wYear,stLocalTime.wMonth,stLocalTime.wDay,stLocalTime.wHour,stLocalTime.wMinute,stLocalTime.wSecond);  
	}
	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 
	
	if(hFile != INVALID_HANDLE_VALUE) 
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei; 
		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 
		MINIDUMP_TYPE mdt       = static_cast<MINIDUMP_TYPE>(MiniDumpNormal |MiniDumpWithDataSegs|MiniDumpWithHandleData); 
		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0 ); 
		
		if( !rv ) 
		{
			_tprintf( _T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError() ); 
		}
		else 
		{
			_tprintf( _T("Minidump created.\n") ); 
		}
		CloseHandle( hFile );
		//UploadDump("191.1.1.23",szFileName);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
