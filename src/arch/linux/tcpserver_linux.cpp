#include "stdafx.h"
#include "tcpserver_linux.h"
#include "looper/looper.h"
#include "tcpclient_linux.h"
#include "net/tcpclient.h"
using namespace std;

namespace Bear {
namespace Core
{
namespace Net {
TcpServer_Linux::TcpServer_Linux()
{
	mSock = INVALID_SOCKET;
	mPort = 0;

	//DV("%s", __func__);
}

TcpServer_Linux::~TcpServer_Linux()
{
	SockTool::CLOSE_SOCKET(mSock);

	//DV("%s", __func__);
}

int TcpServer_Linux::StartServer(int port)
{
	ASSERT(mSock == INVALID_SOCKET);
	mSock = SockTool::StartServer(port);
	SockTool::SetAsync(mSock);

#ifdef _DEBUG
	DV("%s,port=%d,sock=%d", __func__, port, mSock);
#endif

	if (mSock != INVALID_SOCKET)
	{
		mPort = port;

		SOCKET& s = mSock;
		auto handle = GetLooperHandle();
		int ret = -1;
#ifdef __APPLE__
		struct kevent evt = { 0 };
		EV_SET(&evt, s, EVFILT_READ, EV_ADD, 0, 0, (EpollProxy*)this);
		ret = kevent((int)(long)handle, &evt, 1, NULL, 0, NULL);

#else
		struct epoll_event evt = { 0 };
		evt.events = EPOLLIN
			//| EPOLLOUT	//如果加上会一直触发此事件
			| EPOLLRDHUP
			| EPOLLERR
			;

		evt.data.ptr = (EpollProxy*)this;
		ret = epoll_ctl((int)(LRESULT)handle, EPOLL_CTL_ADD, (int)s, &evt);
#endif
		if (ret == 0)
		{
		}
		else
		{
			ASSERT(FALSE);
		}
		return ret;
	}

	return -1;
}

int TcpServer_Linux::OnAccept()
{
	int acceptCount = 0;

	while (1)
	{
		SOCKADDR_IN ca;
		socklen_t caLen = sizeof(ca);
		SOCKET s = accept(mSock, (SOCKADDR*)&ca, &caLen);
		if (s >= 0)
		{
			++acceptCount;

			//DV("accept sock=%d", s);

			SockTool::SetAsync(s);

			auto sp(CreateChannel());
			if (sp)
			{
				AddChild(sp);
				int ret = sp->OnConnect((long)s, nullptr);
				if (ret)
				{
					sp->Destroy();
				}
			}
			else
			{
				SockTool::CLOSE_SOCKET(s);
			}
		}
		else
		{
			break;
		}
	}

	if (acceptCount > 1)
	{
		//DV("accept client count = %d", acceptCount);
	}
	else if (acceptCount == 0)
	{
		DW("accept no more client");
	}

	return 0;
}

std::shared_ptr<Channel> TcpServer_Linux::CreateChannel()
{
	auto sp(make_shared<TcpClient>());
	return sp;
}

LRESULT TcpServer_Linux::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		Stop();
		break;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

void TcpServer_Linux::Stop()
{
	SOCKET &s = mSock;
	if (s != INVALID_SOCKET)
	{

		{
			auto handle = GetLooperHandle();
			int ret = -1;
#ifdef __APPLE__
			DW("todo?");
#else
			struct epoll_event evt = { 0 };
			ret = epoll_ctl((int)(LRESULT)handle, EPOLL_CTL_DEL, (int)s, &evt);//remove all events
#endif
			if (ret)
			{
				DW("Fail %s", __func__);
			}
		}

		shutdown(s, SD_BOTH);
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

void TcpServer_Linux::OnEvent(DWORD events)
{
	//auto objThis = shared_from_this();//确保在OnEvent执行期间不被删除

	//DV("OnEvent,events = 0x%x", events);
	if (events & EPOLLIN)
	{
		OnAccept();
	}
	else
	{
		DW("todo:events=0x%08x", events);
	}
}
}
}
}
