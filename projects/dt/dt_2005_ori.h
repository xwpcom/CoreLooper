//ËµÃ÷:
//DTÊÇÒ»¸öÓÃÓÚµ÷ÊÔµÄÐ¡¹¤¾ß,Æä¹¦ÄÜÀàËÆÓÚVCÖÐµÄTRACE,µ«Ëü¿É½ÓÊÜdebugºÍreleaseÏÂ³ÌÐòµÄÊä³ö,
//²¢ÇÒ²»ÒÀÀµÓÚVC IDE.
//ÆäÊä³öÓÃDT.exe½ÓÊÕ.
//ÓÃÓÚC++Ê±,DTÄÜÊä³öÏûÏ¢ËùÔÚµÄÔ´´úÂëÎÄ¼þ¼°ËùÔÚÐÐ,µ«ÔÚCÖÐ²»Ö§³Ö.

//Ê¹ÓÃ·½·¨:
//Ìí¼Ó±¾ÎÄ¼þµ½¹¤³Ì,ÔÚstdafx.hÖÐ¼ÓÈë
//#include "dt_2005.h"
//Ê¹ÓÃDT,DW,DEÈý¸öºêÊä³öÕï¶ÏÏûÏ¢
//demo:
//DT("Hello,baby,name=%s,age=%d",szName,nAge);
//DW("ÕâÊÇÒ»Ìõ¾¯¸æÏûÏ¢,ÔÚdt.exeÖÐ»áÒÔ·ÛºìÉ«ÏÔÊ¾");
//DE("³ö´íÏûÏ¢,ÔÚdt.exeÖÐ»áÒÔºìÉ«ÏÔÊ¾");
//Ê¹ÓÃ·½·¨ÓëTRACEÍêÈ«ÏàÍ¬.
//×¢Òâ:ÓÉÓÚ¿ÉÄÜËæÊ±½ûÓÃÕâÐ©ºê,ÈçÍ¬TRACEÒ»Ñù,Çë±ÜÃâÔÚDxÓï¾äÖÐÊ¹ÓÃ´ø¸º×÷ÓÃµÄ²Ù×÷.

//2005.12.15Ôö¼ÓONLY_ONCE
//Ê¹ÓÃ·½·¨:
//ONLY_ONCE(DT("ÕâÌõÓï¾äÖ»Ö´ÐÐÒ»´Î"));
//ONLY_ONCE¿ÉÖ´ÐÐ·µ»ØÖµÎªintÀàÐÍµÄfunction,ÓÃÓÚÄ³Ð©Ö»ÐèÒªÖ´ÐÐÒ»´ÎµÄ³¡ºÏ.
//ËüÀàËÆÓÚÈçÏÂ´úÂë:
//ONLY_ONCE(x)=>
//{static BOOL b(x)}

//ÏÞÖÆ:
//¹¤³ÌÀàÐÍÐèÒªÖ§³ÖHWND(linuxÏÂÓÐÏàÓ¦°æ±¾).

//to do
//.¸ÄÓÃpipe»òÀàËÆ·½·¨ÊµÏÖ¿ÉÈ¥µôÐèÒªHWNDµÄÏÞÖÆ.
//.Ôö¼Ó¶ÔCµÄÖ§³Ö.

// by xwpcom at 2005.12.12
// 10moons.com

#ifndef _DT_2005_H_
#define _DT_2005_H_

#include <stdio.h>
//#ifdef __cplusplus

//´íÎó¼¶±ð
#define DT_DISABLE		0x0000	//½ûÖ¹log
#define DT_ERROR		0x0001
#define DT_WARNING		0x0002
#define DT_TRACE		0x0003
#define DT_VERBOSE		0x0004

class _CDT
{
public:
    _CDT(const char* lpszFile, int nLine,int nLevel)
		:m_lpszFile(lpszFile ),m_nLine(nLine),m_nLevel(nLevel)
    {
    }

    int operator()( const char* lpszFormat, ... )
    {
		char szFormat[1024];
		char szMsg[1024*64];
		memset(szMsg,0,sizeof(szMsg));
		_snprintf(szFormat,sizeof(szFormat)-1,"$$@@%s(%d)$$@@%d$$@@%s",m_lpszFile,m_nLine,m_nLevel,lpszFormat);
		lpszFormat = szFormat;
		
		va_list argList;
		va_start(argList, lpszFormat);
		vsprintf(szMsg,lpszFormat, argList);
		va_end(argList);

		//send message to dt.exe
		{
			static HWND hwnd = ::FindWindowEx(NULL,NULL,NULL,"DT ");
			if(!IsWindow(hwnd))
			{
				hwnd = ::FindWindowEx(NULL,NULL,NULL,"DT ");
				if(!hwnd)
					hwnd = ::FindWindow(NULL,"DebugHelper ");//向后兼容,旧版名称
			}

			if(hwnd)
			{
				DWORD dwRet = 0;
				//::SendMessageTimeout(hwnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szMsg,SMTO_BLOCK,10*1000,&dwRet);

				COPYDATASTRUCT cs;
				cs.dwData=0;
				cs.cbData=strlen(szMsg)+1;
				cs.lpData=szMsg;
				::SendMessageTimeout(hwnd,WM_COPYDATA,0,(LPARAM)&cs,SMTO_BLOCK,10*1000,&dwRet);
			}
		}
		return 0;
    }
protected:
    const char* m_lpszFile;
    int m_nLine;
	int m_nLevel;
};

//¶¨Òå_DTÆôÓÃDT,DW,DEÊä³ö
//undef _DT½ûÓÃDT,DW,DEÊä³ö,½ûÓÃºó,ÕâÐ©ºê²»»á¶Ô³ÌÐò²úÉúÈÎºÎÓ°Ïì,Ò²²»»áÏò³ÌÐòÔö¼Ó__FILE__µÈ×Ö·û´®.
#define _DT
//#undef  _DT
#ifdef _DT
	#define DV	(_CDT( __FILE__, __LINE__,DT_VERBOSE))
	#define DT	(_CDT( __FILE__, __LINE__,DT_TRACE))
	#define DW	(_CDT( __FILE__, __LINE__,DT_WARNING))
	#define DE	(_CDT( __FILE__, __LINE__,DT_ERROR))
	#define ONLY_ONCE(x)	\
	do{						\
		static BOOL b(x);	\
	}while(0)
#else
	#define DV	0
	#define DT	0
	#define DW	0
	#define DE	0
	#define ONLY_ONCE	0
#endif

#ifndef COUNT_OF
#define COUNT_OF(x)	(int)((sizeof(x)/sizeof((x)[0])))
#endif

#endif
