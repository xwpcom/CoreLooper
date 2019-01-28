#pragma once

#include "win32.h"
#include <stdarg.h>
#include "shelltool.h"
//XiongWanPing 2014.07.10
namespace Bear {
namespace Core
{

#ifndef _FILE_DT_TAG
#define _FILE_DT_TAG	__FILE__
#endif

enum eDTLevel
{
	DT_MIN = 0,
	DT_DISABLE = 0,
	DT_FATAL,
	DT_ERROR,
	DT_WARNING,
	DT_NOTICE,
	DT_TRACE,
	DT_VERBOSE,
	DT_MAX,
};

class CORE_EXPORT CDT
{
public:
	CDT(const char* lpszFile, int nLine, int nLevel)
		:m_lpszFile(lpszFile), m_nLine(nLine), m_nLevel(nLevel)
	{
	}

	int operator()(const char* lpszFormat, ...);
	static void enableDT(bool enable);
	static bool isDTEnabled()
	{
		return mEnabled;
	}
protected:
	const char* m_lpszFile;
	int m_nLine;
	int m_nLevel;
	static bool mEnabled;
};

#ifdef _DISABLE_DT
#define DV	__noop
#define DT	__noop
#define DW	__noop
#define DE	__noop
#define DF	__noop
#else
//在android下输出在logcat
//在windows下用DT.exe接收,其源码在projects\dt\DebugHelper.sln
//在linux下输出在Terminal或SecureCRT
//在xcode中输出到output中
#define DV	(CDT( __FILE__, __LINE__,DT_VERBOSE))
#define DT	(CDT( __FILE__, __LINE__,DT_TRACE))
#define DG	(CDT( __FILE__, __LINE__,DT_NOTICE))
#define DW	(CDT( __FILE__, __LINE__,DT_WARNING))
#define DE	(CDT( __FILE__, __LINE__,DT_ERROR))
#define DF	(CDT( __FILE__, __LINE__,DT_FATAL))
#endif

#define ONLY_ONCE(x)	\
	do{						\
		static BOOL b(x);	\
	}while(0)

void DumpCallStack();

#ifndef _MSC_VER
#ifdef _DEBUG
#define ASSERT(x)	do{if((x)){}else{DumpCallStack();while(1){DE("###ASSERT Fail:%s (File:[%s:%d])\n",#x,__FILE__,__LINE__);sleep(1000*5);};}}while(0)
#else
#define ASSERT(x)		do{}while(0)
#endif

#define VERIFY(x)	do{if((x)){}else{DE("###VERIFY Fail:%s (File:[%s:%d])\n",#x,__FILE__,__LINE__);}}while(0)
#endif

inline bool ImplicitCastToBool(bool result) { ASSERT(result); return result; }

}
}
