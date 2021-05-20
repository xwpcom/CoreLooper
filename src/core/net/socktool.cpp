#include "stdafx.h"
#include "socktool.h"
#include "base/stringtool.h"
#include "bindwrapper.h"

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {

static const char* TAG = "SockTool";

int SockTool::InitSockTool()
{
#ifdef _MSC_VER
	WSADATA wsaData;
	int nRet = 0;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		LogW(TAG,"Fail WSAStartup");
		return -1;
	}
#endif

	return 0;
}

//socketpair实现linux下的socketpair功能
//socketpair返回两个可以互相通讯的同步tcp socket
//return 0 on success
//return -1 if fail
int SockTool::socketpair(SOCKET& sock0, SOCKET& sock1)
{
	ASSERT(sock0 == INVALID_SOCKET && sock1 == INVALID_SOCKET);
	sock0 = sock1 = INVALID_SOCKET;
#ifdef _MSC_VER	
	SOCKET sListen = INVALID_SOCKET;
	int nPort = 20000;
	while (nPort < 20100)
	{
		sListen = StartServer(nPort);
		if (sListen != INVALID_SOCKET)
			break;
		nPort++;
	}

	if (sListen == INVALID_SOCKET)
	{
		LogW(TAG,"Fail to listen for socketpair");
		return -1;
	}

	CAutoClose ac(&sListen);

	SOCKET s = SockTool::SocketEx(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SockTool::SetAsync(s);
	CAutoClose acs(&s);

	struct sockaddr_in servAddr;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons((unsigned short)nPort);

	struct timeval tval;
	tval.tv_sec = 2;
	tval.tv_usec = 0;
	int ret = connect(s, (struct sockaddr *) &servAddr, sizeof(struct sockaddr));

	fd_set rset, wset;
	FD_ZERO(&rset);
	FD_SET(sListen, &rset);
	FD_SET(s, &rset);
	SOCKET smax = sListen;
	if (s > smax)
	{
		smax = s;
	}
	wset = rset;

	for (int i = 0; i < 3; i++)
	{
		//ret = select(smax+ 1, &rset, &wset, NULL,&tval);
		ret = select((int)(smax + 1), &rset, NULL, NULL, &tval);
		if (FD_ISSET(sListen, &rset))
		{
			SOCKADDR_IN ca;
			socklen_t caLen = sizeof(ca);
			SOCKET sAccept = accept(sListen, (SOCKADDR*)&ca, &caLen);
			//DV("socketpair accept s=%d",s);
			sock0 = sAccept;
			break;
		}
		else
		{

		}
	}

	if (sock0 != INVALID_SOCKET)
	{
		acs.Detach();
		SockTool::SetAsync(s, FALSE);//make s sync
		sock1 = s;
		return 0;
	}

	return -1;
#else
	SOCKET	sock[2];
	int ret = ::socketpair(AF_LOCAL, SOCK_STREAM, 0, sock);

	if (ret == 0)
	{
		sock0 = sock[0];
		sock1 = sock[1];
		SockTool::SetAsync(sock0);
		SockTool::SetAsync(sock1);
		LogV(TAG, "socketpair,s0=%d,s1=%d", sock0, sock1);
	}
	else
	{
		LogW(TAG,"error=%d,%s", errno, strerror(errno));
		ASSERT(FALSE);
	}
	return ret;
#endif
}

BOOL SockTool::IsWouldBlock()
{
	int err = SockTool::GetLastError();
#ifdef _MSC_VER
	return	err == WSAEWOULDBLOCK;
#else
	if (err == 150)
	{
		//在t20上面ftpclient连接有时报connect ret=-1,error=150(Operation now in progress)
		static const char *desc = "Operation now in progress";
		const char *reason = strerror(err);
		//LogV(TAG,"errno=%d(%s)", err, reason);
		if (reason)
		{
			if (strcmp(desc, reason) == 0)
			{
				return TRUE;
			}
		}
	}

	return err == EAGAIN || err == EWOULDBLOCK || err == EINPROGRESS || err == WSAEWOULDBLOCK;
#endif
}

int SockTool::Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	return BindWrapper::Bind(sockfd, (SOCKADDR*)addr, addrlen);
}

int SockTool::StartServer(int port)
{
	SOCKET s = SockTool::SocketEx(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		LogW(TAG,"Fail socket,errno=%d(%s)", errno, strerror(errno));
		return -1;
	}
	SockTool::CAutoClose ac(&s);

	int ret = 0;
#ifdef _MSC_VER	
	char *OptVal = "1";
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, OptVal, sizeof(char));
#else
	int opt = 1;
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
	ASSERT(ret == 0);

	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons((unsigned short)port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = ::bind(s, (SOCKADDR*)&sa, sizeof(sa));
	if (ret == SOCKET_ERROR)
	{
		LogW(TAG,"bind fail,errno=%d(%s),port=%d", errno, strerror(errno), port);
		return -1;
	}

	ret = listen(s, 100);
	if (ret == SOCKET_ERROR)
	{
		LogW(TAG,"listen fail,errno=%d(%s)", errno, strerror(errno));
		return -1;
	}

	ac.Detach();
	return (int)s;
}

void SockTool::CLOSE_SOCKET(SOCKET& s)
{
	if (s != INVALID_SOCKET)
	{
		if (s)
		{
			//DV("closesocket s=%d",s);
			shutdown(s, SD_BOTH);
			closesocket(s);
		}
		else
		{
			//XiongWanPing 2012.09.10
			DT("");
			DT("");
			LogW(TAG,"############################################################### skip close s=%d", s);
			DT("");
			DT("");

			ASSERT(s);
		}

		s = INVALID_SOCKET;
	}
}

SOCKET SockTool::SocketEx(int af, int type, int protocol)
{
	SOCKET s = socket(af, type, protocol);
	if (s == INVALID_SOCKET)
	{
		LogW(TAG,"fail socket,af=%d,type=%d,protocol=0x%x,err=%s", af, type, protocol, GetErrorDesc(GetLastError()));
#if !defined _MSC_VER && !defined __APPLE__
		DT("AF_PACKET=%d,PF_INET=%d,NETLINK_ROUTE=%d,SOCK_STREAM=%d,SOCK_DGRAM=%d,SOCK_PACKET=%d",
			AF_PACKET, PF_INET, NETLINK_ROUTE, SOCK_STREAM, SOCK_DGRAM, SOCK_PACKET);
#endif
	}
	else
	{
		if (af == AF_INET && type == SOCK_DGRAM)// && protocol == IPPROTO_UDP)
		{
			SockTool::SetTimeOut(s, 30, 30);
		}
	}
	//DT("socket s=%d",s);
	return s;
}

int SockTool::CompareIP(const char *ip0, const char *ip1)
{
	int n00, n01, n02, n03;
	int n10, n11, n12, n13;

	int n0 = sscanf(ip0, "%d.%d.%d.%d", &n00, &n01, &n02, &n03);
	int n1 = sscanf(ip1, "%d.%d.%d.%d", &n10, &n11, &n12, &n13);
	if (n0 == 4 && n1 == 4)
	{
#define CMP(x,y)	if(x>y) return 1;else if(x<y) return -1;
		CMP(n00, n10);
		CMP(n01, n11);
		CMP(n02, n12);
		CMP(n03, n13);
		return 0;
#undef CMP
	}

	return 1;
}

int SockTool::SetNoDelay(SOCKET s)
{
	int flag = 1;
	int ret = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	return ret;
}

int SockTool::SetAsync(SOCKET s, bool bAsync)
{
	//DT("%s,s=%d,bAsync=%d", __func__,s,bAsync);

	if (s < 0)
	{
		LogW(TAG,"fail SetAsync,sock=%d", s);
		return -1;
	}

#ifdef _MSC_VER
	u_long value = bAsync ? 1 : 0;
	int ret = ioctlsocket(s, FIONBIO, &value);//make it async
	if (ret)
	{
		int err = SockTool::GetLastError();
		if (err == WSAENOTSOCK)
		{
			LogW(TAG,"fail ioctlsocket,WSAENOTSOCK,maybe have closed sock=%d ", s);
		}
		else
		{
			LogW(TAG,"fail ioctlsocket,error=%d", err);
		}
	}
	//ASSERT(ret==0);
	return ret;
#else
	int ret = 0;
	int flags = fcntl(s, F_GETFL, 0);
	if (bAsync)
	{
		flags |= O_NONBLOCK;//set to noblock mode
	}
	else
	{
		flags &= ~O_NONBLOCK;//unclear
	}
	ret = fcntl(s, F_SETFL, flags);
	if (ret)
	{
		LogW(TAG,"fail F_SETFL,SetAsync(s=%d,bAsync=%d),err=%d(%s)",
			s, bAsync, errno, strerror(errno));
		//ASSERT(FALSE);
	}
	return ret;
#endif
}

int SockTool::GetLastError()
{
#ifdef _MSC_VER
	return WSAGetLastError();
#else
	return errno;
#endif
}

const char * SockTool::GetErrorDesc(int error)
{
#ifdef _MSC_VER
#define ITEM(x)	(char*)x,#x
	static char *pszErr[] =
	{
		ITEM(WSANO_DATA),
		//ITEM(WSAESOCKTNOSUPPOR),


		ITEM(WSAEINTR),
		ITEM(WSAEBADF),
		ITEM(WSAEACCES),
		ITEM(WSAEFAULT),
		ITEM(WSAEINVAL),
		ITEM(WSAEMFILE),

		ITEM(WSAEWOULDBLOCK),
		ITEM(WSAEINPROGRESS),
		ITEM(WSAEALREADY),
		ITEM(WSAENOTSOCK),
		ITEM(WSAEDESTADDRREQ),
		ITEM(WSAEMSGSIZE),
		ITEM(WSAEPROTOTYPE),
		ITEM(WSAENOPROTOOPT),
		ITEM(WSAEPROTONOSUPPORT),
		ITEM(WSAESOCKTNOSUPPORT),
		ITEM(WSAEOPNOTSUPP),
		ITEM(WSAEPFNOSUPPORT),
		ITEM(WSAEAFNOSUPPORT),
		ITEM(WSAEADDRINUSE),
		ITEM(WSAEADDRNOTAVAIL),
		ITEM(WSAENETDOWN),
		ITEM(WSAENETUNREACH),
		ITEM(WSAENETRESET),
		ITEM(WSAECONNABORTED),
		ITEM(WSAECONNRESET),
		ITEM(WSAENOBUFS),
		ITEM(WSAEISCONN),
		ITEM(WSAENOTCONN),
		ITEM(WSAESHUTDOWN),
		ITEM(WSAETOOMANYREFS),
		ITEM(WSAETIMEDOUT),
		ITEM(WSAECONNREFUSED),
		ITEM(WSAELOOP),
		ITEM(WSAENAMETOOLONG),
		ITEM(WSAEHOSTDOWN),
		ITEM(WSAEHOSTUNREACH),
		ITEM(WSAENOTEMPTY),
		ITEM(WSAEPROCLIM),
		ITEM(WSAEUSERS),
		ITEM(WSAEDQUOT),
		ITEM(WSAESTALE),
		ITEM(WSAEREMOTE),

		ITEM(WSASYSNOTREADY),
		ITEM(WSAVERNOTSUPPORTED),
		ITEM(WSANOTINITIALISED),
		ITEM(WSAEDISCON),
		ITEM(WSAENOMORE),
		ITEM(WSAECANCELLED),
		ITEM(WSAEINVALIDPROCTABLE),
		ITEM(WSAEINVALIDPROVIDER),
		ITEM(WSAEPROVIDERFAILEDINIT),
		ITEM(WSASYSCALLFAILURE),
		ITEM(WSASERVICE_NOT_FOUND),
		ITEM(WSATYPE_NOT_FOUND),
		ITEM(WSA_E_NO_MORE),
		ITEM(WSA_E_CANCELLED),
		ITEM(WSAEREFUSED),
		ITEM(ERROR_CONNECTION_REFUSED),

	};

	for (int i = 0; i < sizeof(pszErr) / sizeof(pszErr[0]); i += 2)
	{
		if (error == (ULONGLONG)pszErr[i])
			return pszErr[i + 1];
	}

	if (error != -1)
	{
		LogW(TAG, "Unknown WinSock errcode:0x%x(%d)", error, error);
	}

	return "Unknown WinSock errcode";
#else
	return strerror(error);
#endif
}

string SockTool::GetLocalIP(SOCKET s)
{
	string ip;
	struct sockaddr_in ClientAddr;
	socklen_t ClientAddrLen = sizeof(ClientAddr);
	if (0 == getsockname(s, (struct sockaddr *)&ClientAddr, &ClientAddrLen))
	{
		LPBYTE pIP = (LPBYTE)&ClientAddr.sin_addr;
		ip = Core::StringTool::Format("%d.%d.%d.%d",
			pIP[0] & 0xFF,
			pIP[1] & 0xFF,
			pIP[2] & 0xFF,
			pIP[3] & 0xFF
		);
	}

	return ip;
}

int SockTool::GetLocalPort(SOCKET s)
{
	struct sockaddr_in ClientAddr;
	socklen_t ClientAddrLen = sizeof(ClientAddr);
	if (0 == getsockname(s, (struct sockaddr *)&ClientAddr, &ClientAddrLen))
	{
		int nPort = ntohs(ClientAddr.sin_port);
		return nPort;
	}

	return 0;
}

string SockTool::GetPeerIP(SOCKET s)
{
	char szBuf[100];
	memset(szBuf, 0, sizeof(szBuf));

	struct sockaddr_in ClientAddr;
	socklen_t ClientAddrLen = sizeof(ClientAddr);
	int ret = getpeername(s, (struct sockaddr*)&ClientAddr, &ClientAddrLen);
	if (ret == 0)
	{
		strncpy(szBuf, inet_ntoa(ClientAddr.sin_addr), sizeof(szBuf) - 1);
	}
	else
	{
		//LogW(TAG,"fail getpeername,s=%d,errno=%d(%s)", s, errno, strerror(errno));
	}

	return szBuf;
}

int SockTool::GetPeerPort(SOCKET s)
{
	struct sockaddr_in ClientAddr;
	socklen_t ClientAddrLen = sizeof(ClientAddr);
	if (0 == getpeername(s, (struct sockaddr*)&ClientAddr, &ClientAddrLen))
	{
		int nPort = ClientAddr.sin_port;
		nPort = ntohs((unsigned short)nPort);
		return nPort;
	}
	else
	{
		//LogW(TAG,"Fail to GetPeerPort for socket %d", s);
	}

	return 0;
}

//判断IP地址是否有效
bool SockTool::IsValidIP(const char *pszIP)
{
	if (!pszIP)
		return false;

	int arr[4];
	{
		for (int i = 0; i < 4; i++)
			arr[i] = -1;
	}

	int ret = sscanf(pszIP, "%d.%d.%d.%d", &arr[0], &arr[1], &arr[2], &arr[3]);
	if (ret != 4)
		return false;
	for (int i = 0; i < 4; i++)
	{
		if (arr[i] < 0 || arr[i]>255)
			return false;
	}

	return true;
}


unsigned int SockTool::IP2Int(string ip)
{
	unsigned int ip0, ip1, ip2, ip3;
	int ret = sscanf(ip.c_str(), "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3);
	if (ret == 4)
	{
		return (ip0 & 0xff)
			| ((ip1 & 0xff) << 8)
			| ((ip2 & 0xff) << 16)
			| ((ip3 & 0xff) << 24)
			;
	}

	return 0;
}

string SockTool::Int2IP(unsigned int v)
{
	string ip;
	ip = StringTool::Format("%d.%d.%d.%d",
		v & 0xff,
		(v >> 8) & 0xff,
		(v >> 16) & 0xff,
		(v >> 24) & 0xff
	);

	return ip;
}

//设置socket读写超时,单位:秒

BOOL SockTool::SetTimeOut(SOCKET s, int nSendTimeOut, int nRecvTimeOut)
{
	if (s <= 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

#ifdef _MSC_VER
	if (nRecvTimeOut == -1)
		nRecvTimeOut = nSendTimeOut;

	int nTimeOut = nRecvTimeOut * 1000;
	int nRet = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));
	if (nRet == SOCKET_ERROR)
	{
		LogW(TAG,"setsockopt failed!");// ErrCosd=[%d]", WSAGetLastError());
		return FALSE;
	}

	nTimeOut = nSendTimeOut * 1000;
	nRet = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));
	if (nRet == SOCKET_ERROR)
	{
		LogW(TAG,"setsockopt failed!");// ErrCosd=[%d]", WSAGetLastError());
		return FALSE;
	}

	return TRUE;

#else
	{
		struct timeval timeo = { 10,0 };
		socklen_t len = sizeof(timeo);
		timeo.tv_sec = nSendTimeOut;
		int ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
		if (ret != 0)
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	if (nRecvTimeOut == -1)
		nRecvTimeOut = nSendTimeOut;

	{
		struct timeval timeo = { 10,0 };
		socklen_t len = sizeof(timeo);
		timeo.tv_sec = nRecvTimeOut;
		int ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, len);
		if (ret != 0)
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}
	return TRUE;
#endif
}

int SockTool::ReuseAddr(SOCKET s, bool enable)
{
	int ret = -1;
#ifdef _MSC_VER	
	char *OptVal = enable ? "1" : "0";
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, OptVal, sizeof(char));
#else
	int opt = enable ? 1 : 0;
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

	ASSERT(ret == 0);

	return ret ? -1 : 0;
}

int SockTool::ReusePort(SOCKET s, bool enable)
{
	int ret = -1;
#ifdef _MSC_VER	
	//https://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
	//windows中SO_REUSEADDR相当于linux下的SO_REUSEADDR + SO_REUSEPORT
	ret = ReuseAddr(s, enable);
	//char *OptVal = enable ? "1" : "0";
	//ret = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, OptVal, sizeof(char));
#else

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 0x0200
#endif

	int opt = enable ? 1 : 0;
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

	ASSERT(ret == 0);
	return ret ? -1 : 0;
}

//return 1 if wait ok
//return 0 if wait fail
//return -1 if socket error
int SockTool::WaitSocketMessage(SOCKET sock, int nSecond)
{
	if (sock <= 0)
		return -1;

	fd_set      fdread, fdError;
	FD_ZERO(&fdread);
	FD_ZERO(&fdError);

	struct timeval tv;
	tv.tv_sec = nSecond;
	tv.tv_usec = 0;

	int maxfdp1 = (int)sock + 1;
	FD_SET(sock, &fdread);
	FD_SET(sock, &fdError);

	int ret = select(maxfdp1, &fdread, NULL, &fdError, &tv);

	if (ret == 1)
	{
		if (FD_ISSET(sock, &fdError))
		{
			return -1;
		}
		if (FD_ISSET(sock, &fdread))
		{
			return TRUE;
		}
		return TRUE;
	}

	if (ret == 0)
	{
		return 0;
	}
	if (ret<0)
	{
		return -1;
	}
	return 1;
}

int SockTool::Send(SOCKET s, const LPVOID pData, int cbData)
{
	ASSERT(s>0);
	if (s>0)
	{
		auto ret = (int)send(s, (const char*)pData, cbData, 0);
		return ret;
	}
	else
	{
		LogW(TAG,"invalid socket=%d", s);
	}

	return 0;
}

int SockTool::SetSendBuf(SOCKET s, int bufLen)
{
	int ret = 0;
	int value = 0;

#ifdef _DEBUG
	socklen_t len = sizeof(value);
	ret = getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&value, &len);
	//DV("default SO_SNDBUF=%d",value);
#endif

	value = bufLen;
	ret = setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&value, sizeof(value));
	//DV("set SO_SNDBUF=%d,ret=%d",value,ret);

#ifdef _DEBUG
	value = 0;
	ret = getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&value, &len);
	//DV("new SO_SNDBUF=%d", value);
	if (value != bufLen)
	{
		//ASSERT(FALSE);
		LogW(TAG,"SetSendBuf fail,ret=%d",ret);
	}
#endif

	return ret;
}

int SockTool::SetRecvBuf(SOCKET s, int bufLen)
{
	int ret = 0;
	int value = 0;

#ifdef _DEBUG
	socklen_t len = sizeof(value);
	ret = getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&value, &len);
	//DT("default SO_RCVBUF=%d",value);
#endif

	value = bufLen;
	ret = setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&value, sizeof(value));
	//DV("set SO_RCVBUF=%d,ret=%d",value,ret);

#ifdef _DEBUG
	value = 0;
	ret = getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&value, &len);
	//DT("new SO_RCVBUF=%d",value);
#endif
	if (value != bufLen)
	{
		//ASSERT(FALSE);
		LogW(TAG,"SetRecvBuf fail,ret=%d", ret);
	}

	return ret;
}

}
}
}