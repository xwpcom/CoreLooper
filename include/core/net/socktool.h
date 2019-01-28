#pragma once

#ifdef _MSC_VER
#else
typedef fd_set FD_SET;
typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr SOCKADDR;
//#define SD_SEND SHUT_WR

#ifndef WSAEWOULDBLOCK
	#define WSAEWOULDBLOCK                   10035L
#endif

#endif

namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2016.01.25
class CORE_EXPORT SockTool
{
public:
	static int SetSendBuf(SOCKET s, int bufLen);
	static int SetRecvBuf(SOCKET s, int bufLen);
	static BOOL SetTimeOut(SOCKET s, int nSendTimeOut, int nRecvTimeOut = -1);
	static SOCKET SocketEx(int af, int type, int protocol);
	static int CompareIP(const char *ip0, const char *ip1);
	static int SetAsync(SOCKET s, bool bAsync = true);
	static int SetNoDelay(SOCKET s);
	static int ReuseAddr(SOCKET s, bool enable = true);
	static int ReusePort(SOCKET s, bool enable = true);
	static int WaitSocketMessage(SOCKET sock, int nSecond = 10);
	static int Send(SOCKET s, const LPVOID pData, int cbData);

	static bool IsValidIP(const char *pszIP);
	static bool IsValidIP(std::string ip)
	{
		return 	IsValidIP(ip.c_str());
	}
	static std::string GetPeerIP(SOCKET s);
	static int GetPeerPort(SOCKET s);
	static std::string GetLocalIP(SOCKET s);
	static int GetLocalPort(SOCKET s);

	static int InitSockTool();
	static int StartServer(int nPort);
	static BOOL IsWouldBlock();

	static void SetLastError(int error)
	{
#ifdef _MSC_VER
		WSASetLastError(error);
#else
		errno = error;
#endif
	}
	static int GetLastError();
	static const char * GetErrorDesc(
		int uErrCode
#ifdef _MSC_VER
		= WSAGetLastError()
#else
		= errno
#endif
	);

	static int socketpair(SOCKET& sock0, SOCKET& sock1);
	static void CLOSE_SOCKET(SOCKET& s);

	class CAutoClose
	{
	public:
		CAutoClose(SOCKET* ps)
		{
			m_pSock = ps;
		}
		~CAutoClose()
		{
			if (m_pSock)
			{
				SockTool::CLOSE_SOCKET(*m_pSock);
				m_pSock = NULL;
			}
		}
		SOCKET Detach()
		{
			SOCKET s = INVALID_SOCKET;
			if (m_pSock)
			{
				s = *m_pSock;
			}

			m_pSock = NULL;
			return s;
		}
	protected:
		SOCKET * m_pSock;
	};

	static unsigned int IP2Int(std::string ip);
	static std::string Int2IP(unsigned int v);
};

}
}
}