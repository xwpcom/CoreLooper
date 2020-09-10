#include "stdafx.h"
#include "crashdump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment ( lib, "dbghelp.lib" )

char CCrashDump::m_szPrefix[64];
BOOL CCrashDump::m_bInstalled=FALSE;

//已测试确认，release版不加debug信息，生成的.dmp文件也能定位到bug代码行，前提是生成.exe后代码没修改过
long CCrashDump::StartAutoDump(struct _EXCEPTION_POINTERS *pep)
{
	char	szFileName[_MAX_PATH]	= {0};
	SYSTEMTIME stLocalTime;
	GetLocalTime( &stLocalTime );
	if(m_szPrefix[0])
	{
		sprintf( szFileName,"%s_%d%02d%02d_%02d%02d%02d.dmp",
			m_szPrefix,
			stLocalTime.wYear,stLocalTime.wMonth,stLocalTime.wDay,stLocalTime.wHour,stLocalTime.wMinute,stLocalTime.wSecond);  
	}
	else
	{
		sprintf( szFileName,"%d%02d%02d_%02d%02d%02d.dmp",
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
