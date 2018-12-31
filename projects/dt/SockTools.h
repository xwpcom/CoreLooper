// Tools.h: interface for the CSockTools class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLS_H__EC701C84_A4EB_46AE_B91F_5A82A3BBBD6E__INCLUDED_)
#define AFX_TOOLS_H__EC701C84_A4EB_46AE_B91F_5A82A3BBBD6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>
#include <afxsock.h>
#include <afxmt.h>
#include <process.h>
#include <afxtempl.h>

#ifndef CLOSE_SOCKET
#define CLOSE_SOCKET(s)	do{shutdown(s,SD_BOTH);closesocket(s);s=INVALID_SOCKET;}while(0)
#endif

class CSockTools  
{
public:			
	static BOOL InitWSASocket();
	CSockTools();
	virtual ~CSockTools();

	const static char * GetWinSockErrStr(ULONG uErrCode=-1);
	static CString GetPeerIP(SOCKET s);
	static int GetPeerPort(SOCKET s);
	static int GetLocalPort(SOCKET s);
	static BOOL RecvEx(SOCKET s, LPVOID pBuffer, int cbSize);

	static BOOL IsValidIP(const char *pszIP);
	static CString GetIP(CString szDdnsHostName);

	static int ConnectWithTimeOut(SOCKET s,struct sockaddr * psa,int cbsa,struct timeval *tval);
	static int WaitSocketMessage(SOCKET sock,int nSecond=3);

	static SOCKET ConnectServer(const char *pszHost,int nPort,int nTimeOut=3);

};

#endif // !defined(AFX_TOOLS_H__EC701C84_A4EB_46AE_B91F_5A82A3BBBD6E__INCLUDED_)
