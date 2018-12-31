#include "stdafx.h"
#include "udpclient_linux.h"
#include "../../core/looper/message.inl"

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {
UdpClient_Linux::UdpClient_Linux()
{
	DV("%s,this=%p", __func__, this);
	SetObjectName("BaseUdpClient");
}

UdpClient_Linux::~UdpClient_Linux()
{
	DV("%s,this=%p", __func__, this);
}

int UdpClient_Linux::AttachSocket(SOCKET s)
{
	ASSERT(mSock == -1);
	mSock = s;
	return 0;
}

int UdpClient_Linux::Connect(Bundle& info)
{
	UNUSED(info);

	DW("todo:%s", __func__);
	return -1;
}

void UdpClient_Linux::Close()
{
	DW("todo:%s", __func__);
}

int UdpClient_Linux::Send(LPVOID data, int dataLen)
{
	UNUSED(data);
	UNUSED(dataLen);

	DW("todo:%s", __func__);
	return -1;
}
int UdpClient_Linux::Receive(LPVOID buf, int bufLen)
{
	UNUSED(buf);
	UNUSED(bufLen);

	DW("todo:%s", __func__);
	return -1;
}

int UdpClient_Linux::ConnectHelper(string ip)
{
	UNUSED(ip);
	DW("todo:%s", __func__);
	return -1;
}
void UdpClient_Linux::OnEvent(DWORD events)
{
	//DV("this=0x%08x,%s,sock=%d,events = 0x%x", this, __func__, mSock, events);

#if 1
	if (mWaitFirstEvent)
	{
		mWaitFirstEvent = false;

		int error = -1;
		socklen_t len = sizeof(error);
		getsockopt(mSock, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
		if (error)
		{
			//128,Network is unreachable
			//134(Transport endpoint is not connected)
			if (error != 128 && error != 134)
			{
				DV("%s,addr=[%s],mSock=%d,events=0x%02x,peer(%s:%d)error=%d,%s",
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
			SignalOnConnect(this, error, nullptr, nullptr);
			if (error)
			{
				return;
			}
		}
	}
#endif

	//注意hup可能和in事件一起返回，所以要先处理in事件才能确保接收完整的数据
	if (events & EPOLLIN)
	{
		OnReceive();
	}

	if (events & EPOLLOUT)
	{
		//DV("EPOLLOUT,this=%p",this);
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

//连接成功或失败会调用本接口
int UdpClient_Linux::OnConnect(long handle, Bundle* extraInfo)
{
	UNUSED(handle);
	UNUSED(extraInfo);

	DW("todo:%s", __func__);
	return -1;
}

//有数据可读时会调用本接口
void UdpClient_Linux::OnReceive()
{
	//DW("%s", __func__);
	if (!mPendingData.empty())
	{
		for (auto iter = mPendingData.begin(); iter != mPendingData.end(); ++iter)
		{
			const char *buf = (const char*)(*iter)->GetDataPointer();
			DV("%s", buf);
		}
		mPendingData.clear();
	}

	char buf[4096];
	struct sockaddr_storage from;
	bzero(&from, sizeof(from));
	socklen_t len = sizeof(from);
	auto ret = recvfrom(mSock, buf, sizeof(buf) - 1, 0, (sockaddr*)&from, &len);
	if (ret > 0)
	{
		buf[ret] = 0;
		DV("%s", buf);
	}
	else
	{

	}
}

//可写时会调用本接口
void UdpClient_Linux::OnSend()
{
	DW("todo:%s", __func__);
}

//Close()会调用本接口
void UdpClient_Linux::OnClose()
{
	DW("todo:%s", __func__);
}

LRESULT UdpClient_Linux::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		//Close();
		break;
	}
	case BM_DUMP:
	{
		break;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

void UdpClient_Linux::EnableListenWritable()
{
	mListenWritable = true;

	SOCKET& s = mSock;
	auto handle = GetLooperHandle();
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
	ret = epoll_ctl((int)(LRESULT)handle, EPOLL_CTL_MOD, (int)s, &evt);
	//DV("epoll_ctl,handle=%d,s=%d,ret=%d", handle, s, ret);
#endif
	ASSERT(ret == 0);
	UNUSED(ret);
}

void UdpClient_Linux::DisableListenWritable()
{
	mListenWritable = false;

	SOCKET& s = mSock;
	auto handle = GetLooperHandle();
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
	ret = epoll_ctl((int)(LRESULT)handle, EPOLL_CTL_MOD, (int)s, &evt);
#endif

	if (ret)
	{
		DW("epoll_ctl,handle=%p,s=%d,ret=%d,error=%d(%s)", handle, s, ret, errno, strerror(errno));
	}

}

#if !defined _MSC_VER && 0
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include<netdb.h>
#include <assert.h>

#define MAXBUF 100

int mainUdpClient(int argc, const char *argv[])
{
	const char *server = "127.0.0.1";
	const char *serverport = "1234";
	const char *echostring = "helloworld";

	struct addrinfo client, *servinfo;
	memset(&client, 0, sizeof(client));
	client.ai_family = AF_INET;
	client.ai_socktype = SOCK_DGRAM;
	client.ai_protocol = IPPROTO_UDP;

	if (getaddrinfo(server, serverport, &client, &servinfo) < 0)
	{
		printf("error in getaddrinfo");
		exit(-1);
	}

	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd < 0)
	{
		printf("error in socket create");
		exit(-1);
	}
	char bufmsg[2];
	bufmsg[0] = 0;
	bufmsg[1] = '\0';
	ssize_t numBytes = sendto(sockfd, bufmsg, strlen(bufmsg), 0, servinfo->ai_addr, servinfo->ai_addrlen);
	if (numBytes < 0)
	{
		printf("error in send the data");
	}

	struct sockaddr_storage fromaddr;
	socklen_t fromaddrlen = sizeof(fromaddr);
	char buf[MAXBUF + 1];
	numBytes = recvfrom(sockfd, buf, MAXBUF + 1, 0, (struct sockaddr *)&fromaddr, &fromaddrlen);
	if (buf[0] == 1)
	{
		printf("connected to the server\r\n");
	}

	char  echomsg[1024];
	echomsg[0] = 2;
	char *ptr = echomsg + 1;
	strcpy(ptr, echostring);
	numBytes = 0;
	numBytes = sendto(sockfd, echomsg, strlen(echomsg), 0,
		servinfo->ai_addr, servinfo->ai_addrlen);
	if (numBytes < 0)
	{
		printf("error in send the data");
	}
	numBytes = 0;
	numBytes = recvfrom(sockfd, buf, MAXBUF + 1, 0,
		(struct sockaddr *)&fromaddr, &fromaddrlen);
	if (buf[0] == 2)
	{
		char *str = buf + 1;
		printf("recv msg from server %s\r\n", str);
	}

	numBytes = 0;
	buf[0] = 3;
	numBytes = sendto(sockfd, buf, 2, 0, servinfo->ai_addr, servinfo->ai_addrlen);
	if (numBytes < 0)
	{
		printf("error in send the data");
	}

	freeaddrinfo(servinfo);

	printf("Received:%s \r\n", buf);
	close(sockfd);
	return 0;
}

#endif
}
}
}