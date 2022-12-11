#pragma once

#include <dbghelp.h>
#include <tchar.h>

//YinHao & XiongWanPing 2011.03.28
//CCrashDump用来在程序crash时自动生成dump文件，方便分析出错原因
//用法:在CxxxApp::InitInstance()中声明一个局部变量即可:
//CCrashDump dump("GameSvr");
//生成的dmp文件在程序的当前目录，一般是exe所在目录或vc工程目录(通过IDE调试时)
class CORE_EXPORT CCrashDump
{
public:
	//pszPrefix是dump文件前缀
	CCrashDump(const char *pszPrefix)
	{
		if(!m_bInstalled)
		{
			m_bInstalled=TRUE;
			strncpy_s(m_szPrefix,pszPrefix,sizeof(m_szPrefix)-1);

			SetUnhandledExceptionFilter(StartAutoDump);
		}
	}
	virtual~CCrashDump()
	{
	}
	static long WINAPI StartAutoDump(struct _EXCEPTION_POINTERS *pep);
protected:
	static char m_szPrefix[64];
	static BOOL	m_bInstalled;
};
