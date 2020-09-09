#pragma once

#include <dbghelp.h>
#include <tchar.h>

//YinHao & XiongWanPing 2011.03.28
//CCrashDump�����ڳ���crashʱ�Զ�����dump�ļ��������������ԭ��
//�÷�:��CxxxApp::InitInstance()������һ���ֲ���������:
//CCrashDump dump("GameSvr");
//���ɵ�dmp�ļ��ڳ���ĵ�ǰĿ¼��һ����exe����Ŀ¼��vc����Ŀ¼(ͨ��IDE����ʱ)
class CORE_EXPORT CCrashDump
{
public:
	//pszPrefix��dump�ļ�ǰ׺
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
