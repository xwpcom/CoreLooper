#pragma once

#include <stdarg.h>
#include "core/base/win32.h"
#include "core/base/shelltool.h"
//XiongWanPing 2014.07.10
namespace Bear {
namespace Core
{

#ifndef _FILE_DT_TAG
#define _FILE_DT_TAG	__FILE__
#endif

enum eDTLevel
{
	DT_ERROR=0,
	DT_WARN,
	DT_INFO,
	DT_DEBUG,
	DT_VERBOSE,
};

struct tagLogInfo
{
	const char* mTag = nullptr;
	const char* mFile= nullptr;
	int mLine=0;
	int mLevel=0;

	HWND hwnd=0;
	const char* msg = nullptr;
};

class CORE_EXPORT CLog
{
public:
	CLog(const char* lpszFile, int nLine, int nLevel)
		:m_lpszFile(lpszFile), m_nLine(nLine), m_nLevel(nLevel)
	{
	}

	int operator()(const char* tag, const char* lpszFormat, ...);
protected:
	void send(HWND hwnd, char* msg);

	const char* m_lpszFile;
	int m_nLine;
	int m_nLevel;
};

class CORE_EXPORT CDT
{
public:
	CDT(const char* lpszFile, int nLine, int nLevel)
		:m_lpszFile(lpszFile), m_nLine(nLine), m_nLevel(nLevel)
	{
	}

	//int operator()(const char* tag, const char* lpszFormat, ...);
	int operator()(const char* lpszFormat, ...);
	static void enableDT(bool enable);
	static bool isDTEnabled()
	{
		return mEnabled;
	}
	static void send(tagLogInfo& info);
protected:

	const char* mTag="";
	const char* m_lpszFile;
	int m_nLine;
	int m_nLevel;
	static bool mEnabled;
};

#ifdef _DISABLE_DT
#define DV	__noop
#define DT	__noop
#define DG	__noop
#define DW	__noop
#define DE	__noop

#define LogV	  __noop
#define LogD	  __noop
#define LogI	  __noop
#define LogW	  __noop
#define LogE	  __noop

#else
//在android下输出在logcat
//在windows下用DT.exe接收,其源码在projects\dt\DebugHelper.sln
//在linux下输出在Terminal或SecureCRT
//在xcode中输出到output中
#define DV	(CDT( __FILE__, __LINE__,DT_VERBOSE))
#define DT	(CDT( __FILE__, __LINE__,DT_DEBUG))
#define DG	(CDT( __FILE__, __LINE__,DT_INFO))
#define DW	(CDT( __FILE__, __LINE__,DT_WARN))
#define DE	(CDT( __FILE__, __LINE__,DT_ERROR))

//DX与LogX的区别是LogX支持TAG
#define LogV	(CLog( __FILE__, __LINE__,DT_VERBOSE))
#define LogD	(CLog( __FILE__, __LINE__,DT_DEBUG))
#define LogI	(CLog( __FILE__, __LINE__,DT_INFO))
#define LogW	(CLog( __FILE__, __LINE__,DT_WARN))
#define LogE	(CLog( __FILE__, __LINE__,DT_ERROR))

#endif


#define ONLY_ONCE(x)	\
	do{						\
		static BOOL b(x);	\
	}while(0)

void DumpCallStackX();

#ifndef _MSC_VER
#ifdef _DEBUG
#define ASSERT(x)	do{if((x)){}else{DumpCallStackX();while(1){DE("###ASSERT Fail:%s (File:[%s:%d])\n",#x,__FILE__,__LINE__);sleep(1000*5);};}}while(0)
#else
#define ASSERT(x)		do{}while(0)
#endif

#define VERIFY(x)	do{if((x)){}else{DE("###VERIFY Fail:%s (File:[%s:%d])\n",#x,__FILE__,__LINE__);}}while(0)
#endif

inline bool ImplicitCastToBool(bool result) { ASSERT(result); return result; }

}
}
