// Tools.cpp: implementation of the CSockTools class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SockTools.h"
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#define SHUT_RDWR		SD_BOTH
#ifndef ETIMEDOUT
#define ETIMEDOUT		WSAETIMEDOUT
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSockTools::CSockTools()
{

}

CSockTools::~CSockTools()
{

}

const char * CSockTools::GetWinSockErrStr(ULONG uErrCode)
{
#define ITEM(x)	(char*)x,#x
	static char *pszErr[]=
	{
		ITEM(WSANO_DATA),
		//ITEM(WSAESOCKTNOSUPPOR),


	ITEM(WSAEINTR 		   ),
	ITEM(WSAEBADF 		   ),
	ITEM(WSAEACCES		   ),
	ITEM(WSAEFAULT		   ),
	ITEM(WSAEINVAL		   ),
	ITEM(WSAEMFILE		   ),

	ITEM(WSAEWOULDBLOCK    ),
	ITEM(WSAEINPROGRESS    ),
	ITEM(WSAEALREADY       ),
	ITEM(WSAENOTSOCK       ),
	ITEM(WSAEDESTADDRREQ   ),
	ITEM(WSAEMSGSIZE       ),
	ITEM(WSAEPROTOTYPE     ),
	ITEM(WSAENOPROTOOPT    ),
	ITEM(WSAEPROTONOSUPPORT),
	ITEM(WSAESOCKTNOSUPPORT),
	ITEM(WSAEOPNOTSUPP     ),
	ITEM(WSAEPFNOSUPPORT   ),
	ITEM(WSAEAFNOSUPPORT   ),
	ITEM(WSAEADDRINUSE     ),
	ITEM(WSAEADDRNOTAVAIL  ),
	ITEM(WSAENETDOWN       ),
	ITEM(WSAENETUNREACH    ),
	ITEM(WSAENETRESET      ),
	ITEM(WSAECONNABORTED   ),
	ITEM(WSAECONNRESET     ),
	ITEM(WSAENOBUFS        ),
	ITEM(WSAEISCONN        ),
	ITEM(WSAENOTCONN       ),
	ITEM(WSAESHUTDOWN      ),
	ITEM(WSAETOOMANYREFS   ),
	ITEM(WSAETIMEDOUT      ),
	ITEM(WSAECONNREFUSED   ),
	ITEM(WSAELOOP          ),
	ITEM(WSAENAMETOOLONG   ),
	ITEM(WSAEHOSTDOWN      ),
	ITEM(WSAEHOSTUNREACH   ),
	ITEM(WSAENOTEMPTY      ),
	ITEM(WSAEPROCLIM       ),
	ITEM(WSAEUSERS         ),
	ITEM(WSAEDQUOT         ),
	ITEM(WSAESTALE         ),
	ITEM(WSAEREMOTE        ),

	ITEM(WSASYSNOTREADY        ),
	ITEM(WSAVERNOTSUPPORTED    ),
	ITEM(WSANOTINITIALISED     ),
	ITEM(WSAEDISCON            ),
	ITEM(WSAENOMORE            ),
	ITEM(WSAECANCELLED         ),
	ITEM(WSAEINVALIDPROCTABLE  ),
	ITEM(WSAEINVALIDPROVIDER   ),
	ITEM(WSAEPROVIDERFAILEDINIT),
	ITEM(WSASYSCALLFAILURE     ),
	ITEM(WSASERVICE_NOT_FOUND  ),
	ITEM(WSATYPE_NOT_FOUND     ),
	ITEM(WSA_E_NO_MORE         ),
	ITEM(WSA_E_CANCELLED       ),
	ITEM(WSAEREFUSED           ),


	};

	if(uErrCode == -1)
	{
		uErrCode = WSAGetLastError();
	}

	for(int i=0;i<sizeof(pszErr)/sizeof(pszErr[0]);i+=2)
	{
		if(uErrCode == (ULONG)pszErr[i])
			return pszErr[i+1];
	}

	//DT("Unknown WinSock errcode:0x%x(%d)",uErrCode,uErrCode);
	return "Unknown WinSock errcode";
}

CString CSockTools::GetPeerIP(SOCKET s)
{
	SOCKADDR_IN          ClientAddr;
	int                  ClientAddrLen=sizeof(ClientAddr);
	if(0 == getpeername(s,(SOCKADDR *)&ClientAddr,&ClientAddrLen))
	{
		CString szIP = inet_ntoa(ClientAddr.sin_addr);
		return szIP;
	}
	return "";
}

int CSockTools::GetPeerPort(SOCKET s)
{
	SOCKADDR_IN          ClientAddr;
	int                  ClientAddrLen=sizeof(ClientAddr);
	if(0 == getpeername(s,(SOCKADDR *)&ClientAddr,&ClientAddrLen))
	{
		int nPort = ClientAddr.sin_port;
		nPort = ntohs(nPort);
		return nPort;
	}

	return 0;
}

int CSockTools::GetLocalPort(SOCKET s)
{
	SOCKADDR_IN          ClientAddr;
	int                  ClientAddrLen=sizeof(ClientAddr);
	if(0 == getsockname(s,(SOCKADDR *)&ClientAddr,&ClientAddrLen))
	{
		int nPort = ClientAddr.sin_port;
		nPort = ntohs(nPort);
		return nPort;
	}

	return 0;
}


BOOL CSockTools::InitWSASocket()
{
	WSADATA wsaData;
	int nRet = 0;
	if( (nRet = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0 )
	{
		//DW("Fail to CSockTools::InitWSASocket");
		return FALSE;
	}
	return TRUE;
}

BOOL CSockTools::RecvEx(SOCKET s, LPVOID pBuffer, int cbSize)
{
	char* pBuf = (char*)pBuffer;
	int nRecv = 0;
	while(nRecv<cbSize)
	{
		int ret = recv(s,pBuf+nRecv,cbSize - nRecv,0);
		if(ret<=0)
		{
			//DW("CSockTools::RecvEx fail,err=[%s]",GetWinSockErrStr());
			return FALSE;
		}
		nRecv+=ret;
	}
	return TRUE;
}

//判断IP地址是否有效
BOOL CSockTools::IsValidIP(const char *pszIP)
{   
	short   i,j,l,da[3]={0,0,0};   
	char   str[16   +   1];   
	l   =   strlen(pszIP);   
	if   (l<7)   return   0;   
	j=0;   
	for   (i=0;i<l   ;i++   )   
	{   
		if   (   *(pszIP+i)   =='.')   
		{   
			if   (j<3)   
			{   
				da[j]   =   i;   
				j++;   
			}   
			else   
			{
				return   0;
			}
		}   
		else   if(isdigit(*(pszIP+i))==0)   
		{   
			return   0;   
		}   
	}   
	if   (j!=3)   
	{   
		return   0;   
	}   
	memset(str,   0x00,   sizeof(str));   
	strncpy(str,pszIP,da[0]   -   1);   
	if   (atoi(str)>255)   
	{   
		return   0;   
	}   
	memset(str,   0x00,   sizeof(str));   
	strncpy(str,pszIP   +   da[0]   +   1,   da[1]-da[0]   -1);   
	if   (atoi(str)>255)   
	{   
		return   0;   
	}   
	memset(str,   0x00,   sizeof(str));   
	strncpy(str,pszIP   +   da[1]   +   1,   da[2]-da[1]   -1);   
	if   (atoi(str)>255)   
	{   
		return   0;   
	}   
	memset(str,   0x00,   sizeof(str));   
	strncpy(str,pszIP   +   da[2]   +   1,   l   -   da[2]   -   1);   
	if   (atoi(str)>255)   
	{   
		return   0;   
	}   
	return   1;   
}

//如果szDdnsHostName是有效IP,则直接返回,否则认为是在3322.org申请的域名,解析并返回IP.
//失败时返回空串.
CString CSockTools::GetIP(CString szDdnsHostName)
{
	if(CSockTools::IsValidIP(szDdnsHostName))
		return szDdnsHostName;
	
	HOSTENT *pHostEnt=NULL;
	int      nAdapter = 0;
	struct   sockaddr_in sAddr;
	pHostEnt = gethostbyname(szDdnsHostName);

	if(!pHostEnt)
		return "";
	
	if( pHostEnt->h_addr_list[nAdapter] )
	{
		// pHostEnt->h_addr_list[nAdapter] is the current address in host
		// order.
		
		// Copy the address information from the pHostEnt to a sockaddr_in
		// structure.
		memcpy ( &sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdapter],
			pHostEnt->h_length);
		
		// Output the machines IP Address.
		//DT("Name=[%s],Address=[%s]", pHostEnt->h_name,inet_ntoa(sAddr.sin_addr));
		//SetDlgItemText(IDC_IPADDR_DDNS,inet_ntoa(sAddr.sin_addr));
		CString szIP = inet_ntoa(sAddr.sin_addr);
		return szIP;
		
		nAdapter++;
	}

	return "";
}

//ConnectWithTimeOut支持超时连接
//return 0 if connect ok
//return -1 if fail or timeout
int CSockTools::ConnectWithTimeOut(SOCKET s,struct sockaddr * psa,int cbsa,struct timeval *tval)
{
	int ret = 0;
	//set to noblock mode
	u_long uFlags = 1;
	ioctlsocket(s,FIONBIO,&uFlags);
	//DT("fcntl set to non block = %d",ret);
	ret = connect(s, psa, cbsa);
	if ( ret < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		//DW("connect fail,ret=%d,errDesc=[%s]",ret,CSockTools::GetWinSockErrStr(WSAGetLastError()));
		shutdown(s,SD_BOTH);
		closesocket(s);          
		return (-1);
	}
	//DT("connect#2,ret=%d",ret);
	if(ret == 0)
	{
		//DT("connect complete immediately");
		return 0;
	}

    fd_set rset, wset;
	FD_ZERO(&rset);
    FD_SET(s, &rset);
	wset = rset;

	//DT("select,sizeof(rset)=%d",sizeof(rset));
	if ( (ret = select(s+ 1, &rset, &wset, NULL,tval)) == 0) 
	{
		//DW("select timeout");
		//timeout
		shutdown(s,SHUT_RDWR);
		closesocket(s);          
		errno = ETIMEDOUT;
		return (-1);
	}
	//DT("select#2");

	//fcntl(s, F_SETFL, flags);  //restore flags
	uFlags = 0;
	ioctlsocket(s,FIONBIO,&uFlags);

	if(FD_ISSET(s,&wset))
		return 0;
	return -1;
}

//return 1 if wait ok
//return 0 if wait fail
//return -1 if socket error
int CSockTools::WaitSocketMessage(SOCKET sock,int nSecond)
{
	fd_set      fdread,fdError;
	FD_ZERO(&fdread);
	FD_ZERO(&fdError);

	struct timeval tv;
	tv.tv_sec = nSecond;
	tv.tv_usec=0;

	int maxfdp1 = sock + 1;
	FD_SET(sock, &fdread);
	FD_SET(sock, &fdError);

	//DT("start select socket[%d]",sock);
	int ret = select(maxfdp1, &fdread, NULL,&fdError,  &tv);
	//DT("select return,ret=%d",sock);

	if(ret == 1)
	{
		if(FD_ISSET(sock,&fdError))
		{
			return -1;
		}
		if(FD_ISSET(sock,&fdread))
		{
			return TRUE;
		}
		return TRUE;
	}

	if (ret==0) 
	{
		//DT("Fail to select for get_description");
		return 0;
	}
	if(ret<0)
	{
		//DW("WaitSocketMessage error");
		return -1;
	}
	return 1;
}

//创建socket,连接到pszHost
//如果失败则返回INVALID_SOCKET
SOCKET CSockTools::ConnectServer(const char *pszHost,int nPort,int nTimeOut)
{
	CString szIP = GetIP(pszHost);
	if(szIP.IsEmpty())
	{
		return INVALID_SOCKET;
	}

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in servAddr;
	servAddr.sin_addr.s_addr = inet_addr(szIP);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons (nPort);
	
	int nWaitSecond = nTimeOut;
	struct timeval tval;
	tval.tv_sec = nWaitSecond;
	tval.tv_usec = 0;
	int ret = CSockTools::ConnectWithTimeOut(s,(struct sockaddr *) &servAddr,sizeof (struct sockaddr),&tval);
	if(ret != 0)
	{
		CLOSE_SOCKET(s);
		return INVALID_SOCKET;
	}
	return s;
}

