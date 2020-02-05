#include "stdafx.h"
#include "core/base/shelltool.h"
#include "core/base/dt.h"
#include "core/file/file.h"
#include "bytebuffer.h"

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

#define _CONFIG_DT_2020 //2020 year new dt

#ifdef _CONFIG_DT_2020
enum eType
{
	//有些项目是固定长度，所以不需要用TLV表示
	//eLevel//固定1 bytes
	//ePid,	//固定4 bytes
	//eTid,	//固定4 bytes
	//eLine,//固定4 bytes
	//eEncode,//目前还没用到,todo:字符编码，比如utf8,gb2312
	eAppName=1,
	eTag=2,
	eMsg=3,
	eFile=4,
};

static char gAppName[64];
static WORD gAppNameBytes;
#endif

#ifdef _MSC_VER
int CDT::operator()( const char* lpszFormat, ... )
{
	if(!mEnabled)
	{
		return 0;
	}

#ifdef _CONFIG_DT_2020
	//send message to new dt (2020.02.04)
	{
		const auto title = _T("DT2020 ");
		static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, title);
		if (!IsWindow(hwnd))
		{
			hwnd = ::FindWindowEx(NULL, NULL, NULL, title);
		}

		if (hwnd)
		{

			char szMsg[1024 * 64 * sizeof(char)];

			va_list argList;
			va_start(argList, lpszFormat);
			vsprintf_s(szMsg, sizeof(szMsg) - 1, (char*)lpszFormat, argList);
			va_end(argList);

			if (!gAppName[0])
			{
				char buf[MAX_PATH];
				::GetModuleFileNameA(nullptr, buf, sizeof(buf));
				auto pos = strrchr(buf, '\\');
				if (pos)
				{
					++pos;
					strncpy(gAppName, pos, sizeof(gAppName) - 1);
					{
						auto pos = strchr(gAppName, '.');
						if (pos)
						{
							//sometimes append a 'D' in debug version app name
							//for example: release version is BearStudio.exe
							//debug version is BearStudioD.exe
							//for simplify remove 'D' now
							{
								if (pos > gAppName + 1)
								{
									if (pos[-1] == 'D' && islower(pos[-2]))
									{
										pos[-1] = 0;
									}
								}
							}

							*pos = 0;
						}
					}

					gAppNameBytes = (WORD)strlen(gAppName);
				}
			}

			auto pid = ShellTool::GetCurrentProcessId();
			auto tid = ShellTool::GetCurrentThreadId();

			ByteBuffer box;

			//static length fields
			box.WriteByte((BYTE)m_nLevel);
			box.Write(&pid, sizeof(pid));
			box.Write(&tid, sizeof(tid));
			box.Write(&m_nLine, sizeof(m_nLine));

			//TLV fields
			//T:1 bytes
			//L:2 bytes
			{
				box.WriteByte((BYTE)eAppName);
				box.Write((int)gAppNameBytes);
				box.Write(gAppName, gAppNameBytes);
			}

			{
				box.WriteByte((BYTE)eTag);

				char* tag = "bear";
				box.Write((int)strlen(tag));
				box.Write(tag);
			}

			{
				box.WriteByte((BYTE)eMsg);
				const char* msg = szMsg;
				int len = (int)strlen(msg);
				box.Write(len);
				box.Write(szMsg);
			}

			{
				box.WriteByte((BYTE)eFile);
				box.Write((int)strlen(m_lpszFile));
				box.Write(m_lpszFile);
			}

			DWORD_PTR dwRet = 0;

			const char* msg = szMsg;
			const int len = (int)strlen(msg);

			COPYDATASTRUCT cs;
			cs.dwData = 0;
			cs.cbData = box.length();
			cs.lpData = box.data();
			::SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cs, SMTO_BLOCK, 10 * 1000, (PDWORD_PTR)&dwRet);
		}
	}
#endif
	
	//*
	{
		char szFormat[1024 * sizeof(char)];
		char szMsg[1024 * 64 * sizeof(char)];
		memset(szMsg, 0, sizeof(szMsg));

		const char* tFile = m_lpszFile;
		//_sntprintf_s
		_snprintf(szFormat, sizeof(szFormat) - 1, "$$@@%s(%d)$$@@%d$$@@%s$$@@%5d(%5d)",
			tFile, m_nLine, m_nLevel, lpszFormat,
			ShellTool::GetCurrentProcessId(),
			ShellTool::GetCurrentThreadId()
		);

		lpszFormat = szFormat;

		va_list argList;
		va_start(argList, lpszFormat);
		vsprintf_s(szMsg, sizeof(szMsg) - 1, (char*)lpszFormat, argList);
		va_end(argList);

		//send message to dt.exe
		{
			const auto title = _T("DT ");
			static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, title);
			if (!IsWindow(hwnd))
			{
				hwnd = ::FindWindowEx(NULL, NULL, NULL, title);
			}

			if (hwnd)
			{
				DWORD_PTR dwRet = 0;

				const char* msg = szMsg;
				const int len = (int)strlen(msg);

				COPYDATASTRUCT cs;
				cs.dwData = 0;
				cs.cbData = len + 1;//_tcslen(szMsg)*sizeof(char)+1;
				cs.lpData = (LPVOID)msg;//szMsg;
				::SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cs, SMTO_BLOCK, 10 * 1000, (PDWORD_PTR)&dwRet);
			}
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
	auto len = 0;
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
