#include "stdafx.h"
#include "core/base/shelltool.h"
#include "core/base/dt.h"
#include "core/base/filelogger.h"
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
namespace Bear {
namespace Core {
typedef void (*LogCB)(const char* tag, const char* msg, int level, DWORD threadId, const tagTimeMs& t);
LogCB gLogCB;

/*
目前只在linux下生效
*/
CORE_EXPORT void SetLogCB(LogCB obj)
{
	gLogCB = obj;
}

}
}

using namespace Bear::Core;

void CDT::enableDT(bool enable)
{
	mEnabled = enable;
}

static const char * getFileName(const char* filePath)
{
	if (filePath)
	{
		auto len = strlen(filePath);
		for (int i = len - 1; i > 0; i--)
		{
			auto& ch = filePath[i];
			if (ch == '\\' || ch == '/')
			{
				return filePath + i+1;
			}
		}
	}

	return "";
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

enum eType
{
	//有些项目是固定长度，所以不需要用TLV表示
	//version,版本号,每次修改WM_COPYDATA数据格式后增加版本号
	//eLevel//固定1 bytes
	//ePid,	//固定4 bytes
	//eTid,	//固定4 bytes
	//eLine,//固定4 bytes
	//date:DWORD,20200205
	//time:DWORD,hhmmssMMM

	//eEncode,//目前还没用到,todo:字符编码，比如utf8,gb2312
	eAppName=1,
	eTag=2,
	eMsg=3,
	eFile=4,
};

static char gAppName[64];
static WORD gAppNameBytes;

#ifdef _MSC_VER
static const auto* gTitle = _T("DT2020 ");
int CLog::operator()(const string& tag, const char* lpszFormat, ...)
{
	//send message to new dt (2020.02.04)
	{
		static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		if (!IsWindow(hwnd))
		{
			hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		}

		{
			string msg;

			va_list ap;
			va_start(ap, lpszFormat);
			{
				char szMsg[16*1024];
				int bytes = sizeof(szMsg);
				char* psz = szMsg;
				bool useStack = true;
				auto ret = _vscprintf(lpszFormat, ap);
				if (ret >= sizeof(szMsg)) 
				{
					useStack = false;
					bytes = ret + 1;
					psz = new char[bytes]; 
				}
				vsprintf_s(psz, bytes, lpszFormat, ap);
				msg=psz;
				if (!useStack) {
					delete[]psz;
				}
			}

			va_end(ap);

			{
				tagLogInfo info;
				info.hwnd = hwnd;
				info.msg = msg.c_str();
				info.mFile = m_lpszFile;
				info.mLevel = m_nLevel;
				info.mLine = m_nLine;
				info.mTag = tag.c_str();

				FileLogger::addLog(info);
				CDT::send(info);
			}
		}
	}

	return 0;
}

int CLog::operator()(const char* tag,const char* lpszFormat, ...)
{
	//send message to new dt (2020.02.04)
	{
		static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		if (!IsWindow(hwnd))
		{
			hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		}

		{
			string msg;
			va_list ap;
			va_start(ap, lpszFormat);
			{
				char szMsg[16 * 1024];
				int bytes = sizeof(szMsg);
				char* psz = szMsg;
				bool useStack = true;
				auto ret = _vscprintf(lpszFormat, ap);
				if (ret >= sizeof(szMsg))
				{
					useStack = false;
					bytes = ret + 1;
					psz = new char[bytes];
				}
				vsprintf_s(psz, bytes, lpszFormat, ap);
				msg = psz;
				if (!useStack) {
					delete[]psz;
				}
			}

			va_end(ap);

			{
				tagLogInfo info;
				info.hwnd = hwnd;
				info.msg = msg.c_str();
				info.mFile = m_lpszFile;
				info.mLevel = m_nLevel;
				info.mLine = m_nLine;
				info.mTag = tag;

				FileLogger::addLog(info);
				CDT::send(info);
			}
		}
	}

	return 0;
}

void CDT::send(tagLogInfo& info)
{
	if (!info.hwnd)
	{
		return;
	}

	HWND hwnd = info.hwnd;
	const char* msg=info.msg;

	if (!gAppName[0])
	{
		char buf[MAX_PATH];
		::GetModuleFileNameA(nullptr, buf, sizeof(buf));
		auto pos = strrchr(buf, '\\');
		if (pos)
		{
			++pos;
			strncpy_s(gAppName, pos, sizeof(gAppName) - 1);
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

	auto t = ShellTool::GetCurrentTimeMs();
	DWORD date = t.date();
	DWORD time = t.time() * 1000 + t.ms;

	ByteBuffer box;

	//static length fields
	BYTE version = 1;
	box.WriteByte(version);
	box.WriteByte((BYTE)info.mLevel);
	box.Write(&pid, sizeof(pid));
	box.Write(&tid, sizeof(tid));
	box.Write(&info.mLine, sizeof(info.mLine));
	box.Write(&date, sizeof(date));
	box.Write(&time, sizeof(time));

	//TLV fields
	//T:1 bytes
	//L:2 bytes
	{
		box.WriteByte((BYTE)eAppName);
		box.Write((int)gAppNameBytes);
		box.Write(gAppName, gAppNameBytes);
	}

	if(info.mTag)
	{
		box.WriteByte((BYTE)eTag);

		box.Write((int)strlen(info.mTag));
		box.Write(info.mTag);
	}

	{
		box.WriteByte((BYTE)eMsg);
		int len = (int)strlen(msg);
		box.Write(len);
		box.Write(msg);
	}

	{
		box.WriteByte((BYTE)eFile);
		box.Write((int)strlen(info.mFile));
		box.Write(info.mFile);
	}

	DWORD_PTR dwRet = 0;

	COPYDATASTRUCT cs;
	cs.dwData = 0;
	cs.cbData = box.length();
	cs.lpData = box.data();
	::SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cs, SMTO_BLOCK, 10 * 1000, (PDWORD_PTR)&dwRet);
	//::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cs);

	if (gLogCB)
	{
		/*
		const char *tag,const char *msg,int level,DWORD threadId,const tagTimeMs& t
		*/
		gLogCB(info.mTag, info.msg, info.mLevel, ShellTool::GetCurrentThreadId(), t);
	}

}

int CDT::operator()( const char* lpszFormat, ... )
{
	if (!mEnabled)
	{
		return -1;
	}

	//send message to new dt (2020.02.04)
	{
		static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		if (!IsWindow(hwnd))
		{
			hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
		}
	
		if (hwnd)
		{
			string msg;

			va_list ap;
			va_start(ap, lpszFormat);
			{
				char szMsg[16 * 1024];
				int bytes = sizeof(szMsg);
				char* psz = szMsg;
				bool useStack = true;
				auto ret = _vscprintf(lpszFormat, ap);
				if (ret >= sizeof(szMsg))
				{
					useStack = false;
					bytes = ret + 1;
					psz = new char[bytes];
				}
				vsprintf_s(psz, bytes, lpszFormat, ap);
				msg = psz;
				if (!useStack) {
					delete[]psz;
				}
			}

			va_end(ap);

			tagLogInfo info;
			info.hwnd = hwnd;
			info.msg = msg.c_str();
			info.mFile = m_lpszFile;
			info.mLevel = m_nLevel;
			info.mLine = m_nLine;

			send(info);
		}
	}

	return 0;
}
#else
int CLog::operator()(const char* tag, const char* lpszFormat, ...)
{
	/*
	if (!mEnabled)
	{
		return 0;
	}
	//*/

	if (!tag)
	{
		return -1;
	}

	int errSave = errno;//save for restore.DebugTrace do NOT change errno

	const int nLevel = m_nLevel;
	const char* pszFile = m_lpszFile;
	int nLine = m_nLine;

	//char szFormat[1024*sizeof(char)];
	char szMsg[1024 * 32 * sizeof(char)];
	char buf[1024 * 32 * sizeof(char)];
	memset(szMsg, 0, sizeof(szMsg));
	memset(buf, 0, sizeof(buf));

	char header[100];
	header[0] = 0;
	if (nLevel == DT_WARN)
	{
		strcat(header, "###Warning");
	}
	else if (nLevel == DT_ERROR)
	{
		strcat(header, "###Error  ");
	}

#ifdef _CONFIG_ANDROID
	//AndroidStudio自带时间，所以这里不再添加时间
	auto len = 0;
#else
	tagTimeMs t = ShellTool::GetCurrentTimeMs();
	auto len = strlen(header);
	_snprintf(header + len, sizeof(header) - len - 1,
		"[%04d.%02d.%02d %02d:%02d:%02d.%03u#%04u]"
		, t.year, t.month, t.day, t.hour, t.minute, t.second, t.ms
		, ShellTool::GetCurrentThreadId()
	);
#endif

	char tail[256];
	tail[0] = 0;
	_snprintf(tail, sizeof(tail) - 1, "(%s:%d)", pszFile, nLine);


	va_list varargs;
	va_start(varargs, lpszFormat);
	vsnprintf(buf, sizeof(buf) - 1, lpszFormat, varargs);
	va_end(varargs);

	char* p = szMsg;
	p[0] = 0;

	len = strlen(p);
	_snprintf(p + len, sizeof(szMsg) - len - 1,
		"%s%s#[%s]"
		, header
		, tag
		, buf
	);

	{
		tagLogInfo info;
		//info.hwnd = hwnd;
		info.time = t;
		info.msg = buf;
		info.mFile = getFileName(pszFile);
		info.mLevel = nLevel;
		info.mLine = nLine;
		info.mTag = tag;

		FileLogger::addLog(info);
	}

	bool align = true;//when save log file,maybe disable align

#if defined _CONFIG_ANDROID || defined __APPLE__
	align = false;
#endif

	if (align)
	{
		int maxBodyLen = 100;
		//maxBodyLen = 70;
		auto bodyLen = (int)strlen(szMsg);
		for (int i = 0; i < maxBodyLen - bodyLen; i++)
		{
			len = strlen(p);
			_snprintf(p + len, sizeof(szMsg) - len - 1,
				" "
			);
		}
	}
	len = strlen(p);
	_snprintf(p + len, sizeof(szMsg) - len - 1,
		"%s",
		tail
	);

#ifdef _CONFIG_ANDROID
	{
		android_LogPriority p = ANDROID_LOG_INFO;
		if (nLevel == DT_VERBOSE)
			p = ANDROID_LOG_INFO;
		if (nLevel == DT_WARN)
			p = ANDROID_LOG_WARN;
		else if (nLevel == DT_ERROR)
		{
			p = ANDROID_LOG_ERROR;

			//LogE("Bear/jni", "%s", szMsg);
		}

		{
			//android studio logcat有bug,只能打印一行
			string info(szMsg);
			//info.Replace('\r','~');
			//info.Replace('\n', '`');
			string Tag = StringTool::Format("Bear/jni#%s", tag);
			__android_log_print(p, Tag.c_str(), "%s", szMsg);
		}
	}
#else
	printf("%s\n", szMsg);

	if (gLogCB)
	{
		/*
		const char *tag,const char *msg,int level,DWORD threadId,const tagTimeMs& t
		*/
		gLogCB(tag, buf, m_nLevel, ShellTool::GetCurrentThreadId(), t);
	}

#endif


	errno = errSave;
	return 0;
}

int CLog::operator()(const string& tag, const char* lpszFormat, ...)
{
	/*
	if (!mEnabled)
	{
		return 0;
	}
	//*/

	int errSave = errno;//save for restore.DebugTrace do NOT change errno

	const int nLevel = m_nLevel;
	const char* pszFile = m_lpszFile;
	int nLine = m_nLine;

	//char szFormat[1024*sizeof(char)];
	char szMsg[1024 * 32 * sizeof(char)];
	char buf[1024 * 32 * sizeof(char)];
	memset(szMsg, 0, sizeof(szMsg));
	memset(buf, 0, sizeof(buf));

	char header[100];
	header[0] = 0;
	if (nLevel == DT_WARN)
	{
		strcat(header, "###Warning");
	}
	else if (nLevel == DT_ERROR)
	{
		strcat(header, "###Error  ");
	}

#ifdef _CONFIG_ANDROID
	//AndroidStudio自带时间，所以这里不再添加时间
	auto len = 0;
#else
	tagTimeMs t = ShellTool::GetCurrentTimeMs();
	auto len = strlen(header);
	_snprintf(header + len, sizeof(header) - len - 1,
		"[%04d.%02d.%02d %02d:%02d:%02d.%03u#%04u]"
		, t.year, t.month, t.day, t.hour, t.minute, t.second, t.ms
		, ShellTool::GetCurrentThreadId()
	);
#endif

	char tail[256];
	tail[0] = 0;
	_snprintf(tail, sizeof(tail) - 1, "(%s:%d)", pszFile, nLine);


	va_list varargs;
	va_start(varargs, lpszFormat);
	vsnprintf(buf, sizeof(buf) - 1, lpszFormat, varargs);
	va_end(varargs);

	{
		tagLogInfo info;
		//info.hwnd = hwnd;
		info.time = t;
		info.msg = buf;
		info.mFile = getFileName(pszFile);
		info.mLevel = nLevel;
		info.mLine = nLine;
		info.mTag = tag.c_str();

		FileLogger::addLog(info);
	}

	char* p = szMsg;
	p[0] = 0;

	len = strlen(p);
	_snprintf(p + len, sizeof(szMsg) - len - 1,
		"%s%s#[%s]"
		,header
		,tag.c_str()
		,buf
	);

	bool align = true;//when save log file,maybe disable align

#if defined _CONFIG_ANDROID || defined __APPLE__
	align = false;
#endif

	if (align)
	{
		int maxBodyLen = 100;
		//maxBodyLen = 70;
		auto bodyLen = (int)strlen(szMsg);
		for (int i = 0; i < maxBodyLen - bodyLen; i++)
		{
			len = strlen(p);
			_snprintf(p + len, sizeof(szMsg) - len - 1,
				" "
			);
		}
	}
	len = strlen(p);
	_snprintf(p + len, sizeof(szMsg) - len - 1,
		"%s",
		tail
	);

#ifdef _CONFIG_ANDROID
	{
		android_LogPriority p = ANDROID_LOG_INFO;
		if (nLevel == DT_VERBOSE)
			p = ANDROID_LOG_INFO;
		if (nLevel == DT_WARN)
			p = ANDROID_LOG_WARN;
		else if (nLevel == DT_ERROR)
		{
			p = ANDROID_LOG_ERROR;

			//LogE("Bear/jni", "%s", szMsg);
		}

		{
			//android studio logcat有bug,只能打印一行
			string info(szMsg);
			//info.Replace('\r','~');
			//info.Replace('\n', '`');
			string Tag = StringTool::Format("Bear/jni#%s",tag.c_str());
			__android_log_print(p, Tag.c_str(), "%s", szMsg);
		}
	}
#else
	printf("%s\n", szMsg);

	if (gLogCB)
	{
		/*
		const char *tag,const char *msg,int level,DWORD threadId,const tagTimeMs& t
		*/
		gLogCB(tag.c_str(),buf, m_nLevel, ShellTool::GetCurrentThreadId(),t);
	}

#endif


	errno = errSave;
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
	if(nLevel == DT_WARN)
	{
		strcat(header,"###Warning");
	}
	else if(nLevel == DT_ERROR)
	{
		strcat(header,"###Error  ");
	}

	tagTimeMs t = ShellTool::GetCurrentTimeMs();

#ifdef _CONFIG_ANDROID
	//AndroidStudio自带时间，所以这里不再添加时间
	auto len = 0;
#else
	auto len=strlen(header);
	_snprintf(header+len,sizeof(header)-len-1,
		"[%04d.%02d.%02d %02d:%02d:%02d.%03u#%04u]"
		, t.year, t.month, t.day, t.hour, t.minute, t.second, t.ms
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
		if(nLevel == DT_WARN)
			p=ANDROID_LOG_WARN;
		else if (nLevel == DT_ERROR)
		{
			p = ANDROID_LOG_ERROR;

			//LogE("Bear/jni", "%s", szMsg);
		}

		{
			//android studio logcat有bug,只能打印一行
			string info(szMsg);
			//info.Replace('\r','~');
			//info.Replace('\n', '`');
			__android_log_print(p, "Bear/JNI", "%s", szMsg);
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
