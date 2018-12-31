// by xwpcom at 2005.12.12
// 10moons.com

#ifndef _DT_2005_H_
#define _DT_2005_H_

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#include <stdio.h>
//#ifdef __cplusplus
#ifndef DT_MIN
#define DT_MIN	0
enum eDTLevel
{
	DT_DISABLE	=0,
	DT_FATAL,
	DT_ERROR,
	DT_WARNING,
	DT_NOTICE,
	DT_TRACE,
	DT_VERBOSE,
	DT_MAX,
};
#endif

#include <atlbase.h>
#include <comdef.h>
class _CDT
{
public:
    _CDT(const char* lpszFile, int nLine,int nLevel)
		:m_lpszFile(lpszFile ),m_nLine(nLine),m_nLevel(nLevel)
    {
    }

    int operator()( const TCHAR* lpszFormat, ... )
    {
		TCHAR szFormat[1024*sizeof(TCHAR)];
		TCHAR szMsg[1024*64*sizeof(TCHAR)];
		memset(szMsg,0,sizeof(szMsg));
		
		USES_CONVERSION;
#ifdef _UNICODE
		TCHAR *tFile=A2T(m_lpszFile);
#else
		const char *tFile=m_lpszFile;
#endif
		//_sntprintf(szFormat,sizeof(szFormat)-1,_T("$$@@%s(%d)$$@@%d$$@@%s%s"),tFile,m_nLine,m_nLevel,mCookie,lpszFormat);
		_sntprintf(szFormat,sizeof(szFormat)-1,_T("$$@@%s(%d)$$@@%d$$@@%s$$@@%5d(%5d)"),tFile,m_nLine,m_nLevel,
			lpszFormat,
			GetCurrentProcessId(),
			GetCurrentThreadId()
			);
			lpszFormat = szFormat;
		
		va_list argList;
		va_start(argList, lpszFormat);
		_vstprintf(szMsg,(TCHAR*)lpszFormat, argList);
		va_end(argList);

		//send message to dt.exe
		{
			static HWND hwnd = ::FindWindowEx(NULL,NULL,NULL,_T("DT "));
			if(!IsWindow(hwnd))
			{
				hwnd = ::FindWindowEx(NULL,NULL,NULL,_T("DT "));
				if(!hwnd)
					hwnd = ::FindWindow(NULL,_T("DebugHelper "));//向后兼容,旧版名称
			}

			if(hwnd)
			{
				DWORD dwRet = 0;
				//::SendMessageTimeout(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg,SMTO_BLOCK,10*1000,&dwRet);

				const char *msg=T2A(szMsg);
				const int len=strlen(msg);
				
				COPYDATASTRUCT cs;
				cs.dwData=0;
				cs.cbData=len+1;//_tcslen(szMsg)*sizeof(TCHAR)+1;
				cs.lpData=(LPVOID)msg;//szMsg;
				::SendMessageTimeout(hwnd,WM_COPYDATA,0,(LPARAM)&cs,SMTO_BLOCK,10*1000,&dwRet);
			}
		}
		return 0;
    }
protected:
    const char* m_lpszFile;
    int m_nLine;
	int m_nLevel;
public:
	//static char mCookie[32];
};

/*
char _CDT::mCookie[32];
void SetDTCookie(const char *cookie)
{
	strncpy(_CDT::mCookie,cookie,sizeof(_CDT::mCookie)-1);
}
//*/

#define ONLY_ONCE(x)	\
do{						\
	static BOOL b(x);	\
}while(0)

#ifndef _DISABLE_DT
	#define DV	(_CDT( __FILE__, __LINE__,DT_VERBOSE))
	#define DG	(_CDT( __FILE__, __LINE__,DT_NOTICE))
	#define DT	(_CDT( __FILE__, __LINE__,DT_TRACE))
	#define DW	(_CDT( __FILE__, __LINE__,DT_WARNING))
	#define DE	(_CDT( __FILE__, __LINE__,DT_ERROR))
	#define DF	(_CDT( __FILE__, __LINE__,DT_ERROR))
#else
	#define DV	__noop
	#define DG	__noop
	#define DT	__noop
	#define DW	__noop
	#define DE	__noop
	#define DF	__noop
#endif

#ifndef COUNT_OF
#define COUNT_OF(x)	(int)((sizeof(x)/sizeof((x)[0])))
#endif

#ifndef _countof
#define _countof(x)	(sizeof(x)/sizeof((x)[0]))
#endif
#endif
