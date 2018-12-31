#include "DT_c.h"

#ifdef DEBUG_TRACE

#include <stdio.h>
BOOL DebugTrace(char * lpszFormat,...)
{
	// no MFC used
	static HWND hwnd = ::FindWindow(NULL,"DebugHelper ");
	if(!IsWindow(hwnd))
		hwnd = ::FindWindow(NULL,"DebugHelper ");

	if(hwnd)
	{
		char szMsg[1000];

		va_list argList;
		va_start(argList, lpszFormat);
		try
		{
			vsprintf(szMsg,lpszFormat, argList);
		}
		catch(...)
		{
			strcpy(szMsg ,"DebugHelperÊä³ö×Ö·û´®¸ñÊ½´íÎó!");
		}
		va_end(argList);

		::SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg);
	}
	return TRUE;
}
#endif
