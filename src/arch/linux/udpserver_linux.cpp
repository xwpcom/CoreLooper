#include "stdafx.h"
#include "udpserver_linux.h"
#include "udpclient_linux.h"

namespace Bear {
namespace Core
{
namespace Net {
//SO_REUSEADDR and SO_REUSEPORT
//https://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t
UdpServer_Linux::UdpServer_Linux()
{
	SetObjectName("UdpServer");
}

shared_ptr<UdpClient_Linux> UdpServer_Linux::CreateClient()
{
	if (mClientFactory)
	{
		return mClientFactory->CreateClient();
	}

	return make_shared<UdpClient_Linux>();
}

int UdpServer_Linux::StartServer(int port)
{
	ASSERT(IsMyselfThread());

	ASSERT(mSock == INVALID_SOCKET);
	auto s = SockTool::SocketEx(AF_INET, SOCK_DGRAM, 0);

	SockTool::ReuseAddr(s);
	SockTool::SetAsync(s);

	sockaddr_in my_addr = { 0 };
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	auto ret = ::bind(s, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	ASSERT(ret == 0);
	UNUSED(ret);

	mSock = s;
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

void UdpServer_Linux::OnEvent(DWORD events)
{
	//DV("this=0x%08x,%s,sock=%d,events = 0x%x", this, __func__, mSock, events);

	if (events & EPOLLIN)
	{
		OnAccept();
	}
	else
	{
		DW("todo:events=0x%08x", events);
	}
}

int UdpServer_Linux::OnAccept()
{
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	char buf[1024 * 4];
	auto ret = recvfrom(mSock, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_size);
	//check(ret > 0, "recvfrom error");
	if (ret > 0 && ret < (int)sizeof(buf) - 1)
	{
		buf[ret] = 0;
	}

#ifdef _MSC_VER
	return -1;
#else
	auto data = make_shared<ByteBuffer>();
	data->Write(buf, (int)ret);
	data->MakeSureEndWithNull();

	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	ret = getnameinfo((struct sockaddr *)&client_addr, addr_size, hbuf, sizeof(hbuf), \
		sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	//check(ret == 0, "getnameinfo");

	DV("recv[%s:%s]:[%s]", hbuf, sbuf, buf);
	return OnNewConnect((struct sockaddr_in*)&client_addr, data);
#endif
}

int UdpServer_Linux::OnNewConnect(struct sockaddr_in  *addr, shared_ptr<ByteBuffer> box)
{
	string item = StringTool::Format("%s:%d", inet_ntoa(addr->sin_addr), addr->sin_port);

	ULONGLONG ipPort = ((ULONGLONG)addr->sin_addr.s_addr << 32) | addr->sin_port;
	{
		auto iter = mClients.find(ipPort);
		if (iter != mClients.end())
		{
			auto obj = iter->second.lock();
			if (obj)
			{
				obj->AddPendingData(box);
				obj->OnReceive();
				return 0;
			}
		}
	}

	SOCKET s = SockTool::SocketEx(AF_INET, SOCK_DGRAM, 0);

	SockTool::ReuseAddr(s);
	SockTool::ReusePort(s);
	SockTool::SetAsync(s);

	struct sockaddr_in my_addr = { 0 };
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(mPort);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	if (::bind(s, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
	{
		DW("fail bind");
		return -1;
	}

	auto ret = connect(s, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	DV("udp bind connect %s:%d,peer port=%d,connect ret=%d"
		, inet_ntoa(addr->sin_addr)
		, addr->sin_port
		, SockTool::GetPeerPort(s)
		, ret
	);

	auto client = CreateClient();
	if (!client)
		{
		ASSERT(FALSE);
		SockTool::CLOSE_SOCKET(s);
		return -1;
	}

	client->AttachSocket(s);
	client->AddPendingData(box);
	AddChild(client);
	mClients[ipPort] = client;

	auto handle = GetLooperHandle();
#ifdef __APPLE__
	struct kevent evt = { 0 };
	EV_SET(&evt, s, EVFILT_READ, EV_ADD, 0, 0, (EpollProxy*)client.get());
	ret = kevent((int)(long)handle, &evt, 1, NULL, 0, NULL);

#else
	struct epoll_event evt = { 0 };
	evt.events = EPOLLIN
		//| EPOLLOUT	//如果加上会一直触发此事件
		| EPOLLRDHUP
		| EPOLLERR
		;

	evt.data.ptr = (EpollProxy*)client.get();
	ret = epoll_ctl((int)(LRESULT)handle, EPOLL_CTL_ADD, (int)s, &evt);
#endif
	if (ret == 0)
	{
		DV("epoll_ctl ret=%d", ret);
	}
	else
	{
		ASSERT(FALSE);
	}

	return ret;
}

#if 0
//#ifndef _MSC_VER

#define ECHO_LEN	1025
#define MAXBUF 1024

#define NI_MAXHOST  1025
#define NI_MAXSERV	32

static unsigned int myport = 1234;

static void add_event(int epollfd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void do_write(int epollfd, int fd, char *buf)
{
	auto nwrite = _write(fd, buf, (int)strlen(buf));
	if (nwrite == -1)
	{
		perror("write error:");
		_close(fd);
		delete_event(epollfd, fd, EPOLLOUT);
	}
}

int udp_socket_connect(int epollfd, struct sockaddr_in  *servaddr)
{
	struct sockaddr_in my_addr;
	int fd = (int)SockTool::SocketEx(AF_INET, SOCK_DGRAM, 0);

	SockTool::ReuseAddr(fd);
	SockTool::ReusePort(fd);
	SockTool::SetAsync(fd);
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(myport);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}
	else
	{
		printf("IP and port bind success \r\n");
	}
	if (fd == -1)
		return  -1;

	DV("udp bind connect %s:%d,peer port=%d"
		, inet_ntoa(servaddr->sin_addr)
		, servaddr->sin_port
		, SockTool::GetPeerPort(fd)
	);
	connect(fd, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in));
	add_event(epollfd, fd, EPOLLIN);
	return fd;

}

void accept_client(int epollfd, int fd)
{
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	char buf[1024];
	auto ret = recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&client_addr, &addr_size);
	//check(ret > 0, "recvfrom error");
	buf[ret] = '\0';
	char type = buf[0];
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
#ifndef _MSC_VER
	ret = getnameinfo((struct sockaddr *)&client_addr, addr_size, hbuf, sizeof(hbuf), \
		sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
#endif
	//check(ret == 0, "getnameinfo");

	printf("recvfrom client [%s:%s] : %d\n", hbuf, sbuf, buf[0]);

	if (type != 0)
	{
		return;
	}
	int new_sock = udp_socket_connect(epollfd, (struct sockaddr_in*)&client_addr);
	buf[0] = 1;
	do_write(epollfd, new_sock, buf);
}

void msg_process(int epollfd, int fd)
{
	DV("%s", __func__);

	int nread = 0;
	char buf[MAXBUF];
	char type;
	nread = _read(fd, buf, MAXBUF);
	//check(nread > 0, "recvfrom error");
	buf[nread] = '\0';
	type = buf[0];
	if (type == 2)
	{
		printf("recv msg %s \r\n", buf + 1);
		do_write(epollfd, fd, buf);
	}
	else {
		delete_event(epollfd, fd, EPOLLOUT);
	}

}

int mainUdpServer()
{
	auto listener = socket(AF_INET, SOCK_DGRAM, 0);
	DV("listener=%d", listener);
	SockTool::ReuseAddr(listener);
	SockTool::SetAsync(listener);

	sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(myport);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	auto ret = bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	ASSERT(ret == 0);

	socklen_t len;
	struct epoll_event ev;
	struct epoll_event events[100];

	auto kdpfd = epoll_create(1);
	len = sizeof(struct sockaddr_in);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = (int)listener;
	ret = epoll_ctl(kdpfd, EPOLL_CTL_ADD, (int)listener, &ev);
	if (ret < 0)
	{
		DW("epoll fail");
		return -1;
	}


	while (1)
	{
		/* 等待有事件发生 */
		auto nfds = epoll_wait(kdpfd, events, 10000, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
			break;
		}

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == listener)
			{
				DV("accept");
				accept_client((int)kdpfd, (int)listener);
			}
			else
			{
				msg_process(kdpfd, events[n].data.fd);
			}
		}
	}
	_close((int)listener);
	return 0;
}
#endif 

//#endif
}
}
}
