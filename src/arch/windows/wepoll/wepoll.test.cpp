#include "stdafx.h"
#include "CppUnitTest.h"

#include "wepoll.h"

using namespace std;

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

namespace Wepoll{
	
class EpollProxy
{
public:
	EpollProxy()
	{
		mSock = INVALID_SOCKET;
	}
	EpollProxy(SOCKET s)
	{
		ASSERT(s != INVALID_SOCKET);
		mSock = s;
	}

	virtual ~EpollProxy()
	{
	}
	virtual void OnEvent(DWORD events)
	{
		LogV("EpollProxy", "this=0x%08x,%s, sock=%d, events=0x%x", this, __func__, mSock, events);
	}

protected:
	SOCKET mSock = -1;
};

class TcpClient:public EpollProxy
{
public:
	TcpClient()
	{
		mSock = INVALID_SOCKET;
		mServerSide = true;
		mWaitFirstEvent = true;
		mListenWritable = false;

	}
	~TcpClient()
	{
		SockTool::CLOSE_SOCKET(mSock);
	}
	int Connect(HANDLE hEpoll,const string& ip, int port);
	void OnEvent(DWORD events);
	void EnableListenWritable();
	void DisableListenWritable();
	void MarkEndOfRecv();
	void MarkEndOfSend();
	void OnSend();
	void OnReceive();
	void OnConnectAck(int error);

	void Close();
protected:
	string mTag = "TcpClient";
	string mAddress;

	bool	mServerSide=true;//为true被动连接,为false时表示主动连接
	bool	mListenWritable=false;
	bool    mWaitFirstEvent = true;
	bool	mMarkEndOfSend = false;
	HANDLE mLooperHandle = INVALID_HANDLE_VALUE;
};
	
int TcpClient::Connect(HANDLE hEpoll, const string& ip,int port)
{
	mLooperHandle = hEpoll;
	mAddress = ip;
	mServerSide = false;
	mWaitFirstEvent = true;

	SOCKET s = SockTool::SocketEx(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SockTool::SetAsync(s);
	mSock = s;

	int ret = -1;
	struct epoll_event evt = { 0 };
	evt.events = EPOLLIN
		| EPOLLOUT	//如果加上会一直触发此事件
		| EPOLLRDHUP
		| EPOLLERR
		;

	evt.data.ptr = (EpollProxy*)this;
	ret = epoll_ctl(hEpoll, EPOLL_CTL_ADD, (int)s, &evt);
	ASSERT(ret == 0);

	struct sockaddr_in servAddr;
	servAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	ret = connect(s, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));

	if (ret && SockTool::IsWouldBlock())
	{
		ret = 0;
	}

	return ret;
}

void TcpClient::OnEvent(DWORD events)
{
	//LogV(mTag,"%s,mSock=%d,events=0x%02x",__func__,mSock,events);

	if (mWaitFirstEvent)
	{
		mWaitFirstEvent = false;

		int error = -1;
		socklen_t len = sizeof(error);
		getsockopt(mSock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error)
		{
			//128,Network is unreachable
			//134(Transport endpoint is not connected)
			//148 no route to host
			if (error != 128 && error != 134 && error != 148)
			{
				LogW(mTag, "%s,addr=[%s],mSock=%d,events=0x%02x,peer(%s:%d)error=%d,%s",
					__func__
					, mAddress.c_str()
					, mSock
					, events
					, SockTool::GetPeerIP(mSock).c_str()
					, SockTool::GetPeerPort(mSock)
					, error
					, strerror(error)
				);
			}
		}

		if (!mServerSide)
		{
			/*
			if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				error = -1;
			}
			 //*/

			if (error == 0)
			{
				DisableListenWritable();
			}

			//客户端主动连接，第一次收到可写事件时，触发连接成功或失败事件
			OnConnectAck(error);
			if (error)
			{
				return;
			}
		}
	}

	//注意hup可能和in事件一起返回，所以要先处理in事件才能确保接收完整的数据
	if (events & EPOLLIN)
	{
		OnReceive();
	}

	if (events & EPOLLOUT)
	{
		//LogV(mTag,"EPOLLOUT,this=%p",this);
		if (mListenWritable)
		{
			DisableListenWritable();
		}

		OnSend();
	}

	if (events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP))
	{
		Close();
		return;
	}

}

void TcpClient::EnableListenWritable()
{
	//LogV(mTag, "%s(%p),sock=%d", __func__, this, mSock);

	mListenWritable = true;

	SOCKET& s = mSock;
	auto handle = mLooperHandle;
	int ret = -1;

#ifdef __APPLE__
	struct kevent evt = { 0 };
	EV_SET(&evt, s, EVFILT_WRITE, EV_ADD, 0, 0, (EpollProxy*)this);

	ret = kevent(handle, &evt, 1, NULL, 0, NULL);
#else
	struct epoll_event evt = { 0 };
	evt.events = EPOLLIN
		| EPOLLOUT	//如果加上会一直触发此事件
		| EPOLLRDHUP
		| EPOLLERR
		;

	evt.data.ptr = (EpollProxy*)this;
	ret = epoll_ctl(handle, EPOLL_CTL_MOD, (int)s, &evt);
	//LogV(mTag,"epoll_ctl,handle=%d,s=%d,ret=%d", handle, s, ret);
#endif
	ASSERT(ret == 0);
	if (ret)
	{
		LogW(mTag, "%s fail", __func__);
	}
}

void TcpClient::DisableListenWritable()
{
	//LogV(mTag, "%s(%p),sock=%d", __func__, this, mSock);

	mListenWritable = false;

	SOCKET& s = mSock;
	auto handle = mLooperHandle;
	int ret = -1;

#ifdef __APPLE__
	struct kevent evt = { 0 };
	EV_SET(&evt, s, EVFILT_WRITE, EV_DISABLE, 0, 0, (EpollProxy*)this);

	ret = kevent(handle, &evt, 1, NULL, 0, NULL);
#else
	struct epoll_event evt = { 0 };
	evt.events = EPOLLIN
		//| EPOLLOUT	//如果加上会一直触发此事件
		| EPOLLRDHUP
		| EPOLLERR
		;

	evt.data.ptr = (EpollProxy*)this;
	ret = epoll_ctl(handle, EPOLL_CTL_MOD, (int)s, &evt);
#endif

	if (ret)
	{
		LogW(mTag, "epoll_ctl,handle=%d,s=%d,ret=%d,error=%d(%s)", handle, s, ret, errno, strerror(errno));
	}

}

void TcpClient::MarkEndOfRecv()
{
	shutdown(mSock, SD_RECEIVE);
}

void TcpClient::MarkEndOfSend()
{
	mMarkEndOfSend = true;
	shutdown(mSock, SD_SEND);
}

//当可写时会调用本接口
void TcpClient::OnSend()
{
	//SignalOnSend(this);
}

void TcpClient::OnReceive()
{
	//SignalOnReceive(this);
	char buf[4096];
	auto ret = recv(mSock, buf, sizeof(buf) - 1, 0);
	if (ret > 0)
	{
		buf[ret] = 0;

		LogV(mTag, "recv[%s]",buf);
	}
}

void TcpClient::Close()
{
	//ASSERT(IsMyselfThread());

	bool needFireEvent = false;
	if (mSock != INVALID_SOCKET)
	{
		{
			auto handle = mLooperHandle;
			int ret = -1;
#ifdef __APPLE__
			LogW(TAG, "todo?");
#else
			struct epoll_event evt = { 0 };
			ret = epoll_ctl(mLooperHandle, EPOLL_CTL_DEL, (int)mSock, &evt);//remove all events
#endif

			if (ret)
			{
				LogW(mTag, "fail");
			}
		}

		shutdown(mSock, SD_BOTH);
		closesocket(mSock);
		mSock = INVALID_SOCKET;
		needFireEvent = true;
	}

	if (needFireEvent)
	{
		//OnClose();
	}

	//Destroy();
}

void TcpClient::OnConnectAck(int error)
{
	if (error == 0)
	{
		auto req = StringTool::Format(
			"GET /index.html HTTP/1.1\r\n"
			"\r\n"
		);

		auto ret=send(mSock, req.c_str(), req.length(), 0);
		LogV(mTag, "send ret=%d", ret);
	}
}

/*
XiongWanPing 2022.11.27
测试wepoll,它用到了windows AFD
发现网上找不到AFD相关资料，感觉是microsoft没有开放出来

*/
TEST_CLASS(Wepoll_)
{
public:
	TEST_METHOD(wepoll)
	{
		string mTag = "wepoll";

		auto handle = epoll_create(1);
		LogV(mTag, "handle=%p",handle);

		TcpClient client;
		client.Connect(handle, "47.106.193.63", 80);

		while (1)
		{
			epoll_event events[1024];
			int ms = 10 * 1000;
			int ret = epoll_wait(handle, events, COUNT_OF(events), ms);
			LogV(mTag, "epoll_wait ret=%d", ret);
			if (ret > 0)
			{
				for (int i = 0; i < ret; i++)
				{
					if (events[i].data.ptr)
					{
						EpollProxy* proxy = (EpollProxy*)events[i].data.ptr;
						auto evt = events[i].events;
						proxy->OnEvent(evt);
					}
				}
			}
		}




	}


};
}
