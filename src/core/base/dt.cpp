#include "stdafx.h"
#include "base/shelltool.h"
#include "base/dt.h"
#include "file/file.h"

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;

#ifdef _CONFIG_ANDROID
extern "C"
{
#include <android/log.h>

}
#endif

bool CDT::mEnabled=true;

void CDT::enableDT(bool enable)
{
	mEnabled = enable;
}


#ifdef _CONFIG_ANDROID
extern "C"
{
#include <android/log.h>

}
#endif

#if 1

#if defined _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

#ifdef _MSC_VER
int CDT::operator()( const char* lpszFormat, ... )
{
	if(!mEnabled)
	{
		return 0;
	}
//*
	char szFormat[1024*sizeof(char)];
	char szMsg[1024*64*sizeof(char)];
	memset(szMsg,0,sizeof(szMsg));

	const char *tFile=m_lpszFile;
	//_sntprintf_s
	_snprintf(szFormat, sizeof(szFormat) - 1,"$$@@%s(%d)$$@@%d$$@@%s$$@@%5d(%5d)",
		tFile,m_nLine,m_nLevel,lpszFormat,
		ShellTool::GetCurrentProcessId(),
		ShellTool::GetCurrentThreadId()
		);

	lpszFormat = szFormat;

	va_list argList;
	va_start(argList, lpszFormat);
	vsprintf_s(szMsg,sizeof(szMsg)-1,(char*)lpszFormat, argList);
	va_end(argList);

	//send message to dt.exe
	{
		static HWND hwnd = ::FindWindowEx(NULL,NULL,NULL,_T("DT "));
		if(!IsWindow(hwnd))
		{
			hwnd = ::FindWindowEx(NULL,NULL,NULL,_T("DT "));
		}

		if(hwnd)
		{
			DWORD_PTR dwRet = 0;

			const char *msg=szMsg;
			const int len=(int)strlen(msg);

			COPYDATASTRUCT cs;
			cs.dwData=0;
			cs.cbData=len+1;//_tcslen(szMsg)*sizeof(char)+1;
			cs.lpData=(LPVOID)msg;//szMsg;
			::SendMessageTimeout(hwnd,WM_COPYDATA,0,(LPARAM)&cs,SMTO_BLOCK,10*1000,(PDWORD_PTR)&dwRet);
		}
	}
	//*/
	return 0;
}
#else

int AppendLogToFile(const char *pszLogFile,const char *msg)
{
	auto dwLen=File::GetFileLength(pszLogFile);
	size_t maxFileLen=64*1024;
#ifdef _CONFIG_ANDROID
	maxFileLen = 1024 * 1024;
#endif

	if(dwLen>=maxFileLen)
	{
		File::DeleteFile(pszLogFile);
	}

	FILE *hFile=fopen(pszLogFile,"a+");
	if(hFile)
	{
		auto len=strlen(msg);
		auto ret=fwrite(msg,1,len,hFile);
		if(ret!=len)
		{
			ASSERT(FALSE);
		}

		fwrite("\r\n",1,2,hFile);

		fflush(hFile);
		fclose(hFile);
	}

	return 0;
}

int CDT::operator()( const char* lpszFormat, ... )
{
	if(!mEnabled)
	{
		return 0;
	}

	int errSave = errno;//save for restore.DebugTrace do NOT change errno

	const int nLevel = m_nLevel;
	const char *pszFile=m_lpszFile;
	int nLine=m_nLine;

	//char szFormat[1024*sizeof(char)];
	char szMsg[1024*32*sizeof(char)];
	char buf[1024*32*sizeof(char)];
	memset(szMsg,0,sizeof(szMsg));
	memset(buf,0,sizeof(buf));

	char header[100];
	header[0]=0;
	if(nLevel == DT_WARNING)
	{
		strcat(header,"###Warning");
	}
	else if(nLevel == DT_ERROR)
	{
		strcat(header,"###Error  ");
	}
	else if(nLevel == DT_FATAL)
	{
		strcat(header,"###LOG    ");
	}

#ifdef _MSC_VER
	SYSTEMTIME st;
	GetLocalTime(&st);
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

#ifdef _CONFIG_ANDROID
	//AndroidStudio自带时间，所以这里不再添加时间
#else
	auto len=strlen(header);
	_snprintf(header+len,sizeof(header)-len-1,
		"[%04d.%02d.%02d %02d:%02d:%02d.%03u#%04u]"
		, year, month, day, hour, minute, second, ms
		, ShellTool::GetCurrentThreadId()
		);
#endif

	char tail[256];
	tail[0]=0;
	_snprintf(tail,sizeof(tail)-1,"(%s:%d)",pszFile,nLine);


	va_list varargs;
	va_start (varargs, lpszFormat);
	vsnprintf(buf,sizeof(buf)-1,lpszFormat, varargs);
	va_end (varargs);

	char *p=szMsg;
	p[0]=0;

	len=strlen(p);
	_snprintf(p+len,sizeof(szMsg)-len-1,
		"%s[%s]",
		header,
		buf
		);

	bool align=true;//when save log file,maybe disable align

#if defined _CONFIG_ANDROID || defined __APPLE__
	align=false;
#endif

	if(align)
	{
		int maxBodyLen=100;
		//maxBodyLen = 70;
		auto bodyLen=(int)strlen(szMsg);
		for(int i=0;i<maxBodyLen - bodyLen;i++)
		{
			len=strlen(p);
			_snprintf(p+len,sizeof(szMsg)-len-1,
				" "
				);
		}
	}
	len=strlen(p);
	_snprintf(p+len,sizeof(szMsg)-len-1,
		"%s",
		tail
		);

#ifdef _CONFIG_ANDROID
	{
		android_LogPriority p=ANDROID_LOG_INFO;
		if(nLevel == DT_VERBOSE)
			p=ANDROID_LOG_INFO;
		if(nLevel == DT_WARNING )
			p=ANDROID_LOG_WARN;
		else if (nLevel == DT_ERROR || nLevel == DT_FATAL)
		{
			p = ANDROID_LOG_ERROR;

			Log::e("Bear/jni", "%s", szMsg);
		}

		{
			//android studio logcat有bug,只能打印一行
			string info(szMsg);
			//info.Replace('\r','~');
			//info.Replace('\n', '`');
			__android_log_print(p, "Bear/JNI", "%s", szMsg);

			//AppendLogToFile("/storage/sdcard1/0Camera/ds.log",info);
			
		}
	}
#else
	printf("%s\n",szMsg);
#endif


	errno = errSave;
	return 0;
}

#endif

#endif
