#pragma once
#include "looper/looperimpl.h"

#ifdef _MSC_VER
typedef  DWORD pthread_t;
#endif

#ifdef __APPLE__
#include <stdio.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#endif

namespace Bear {
namespace Core {
class EpollProxyDummy;
#if defined  _MSC_VER || defined __APPLE__

#define EPOLLIN        0x001
#define EPOLLPRI       0x002
#define EPOLLOUT       0x004
#define EPOLLRDNORM    0x040
#define EPOLLRDBAND    0x080
#define EPOLLWRNORM    0x100
#define EPOLLWRBAND    0x200
#define EPOLLMSG       0x400
#define EPOLLERR       0x008
#define EPOLLHUP       0x010

#define EPOLLET        0x80000000
#define EPOLLONESHOT   0x40000000

#define EPOLL_CTL_ADD  1
#define EPOLL_CTL_DEL  2
#define EPOLL_CTL_MOD  3

typedef union epoll_data
{
	void    *ptr;
	int      fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event {
	uint32_t     events;    /* Epoll events */
	epoll_data_t data;      /* User data variable */
};

#else
#include <sys/epoll.h>
#include <sys/syscall.h>
#endif

#ifndef EPOLLRDHUP
#define EPOLLRDHUP     0x2000		//android下也没有定义
#endif


#if defined  _MSC_VER
long epoll_create(int);
long epoll_ctl(int epfd, int op, int sock, epoll_event *);
long epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
#endif

class CORE_EXPORT EpollProxy
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
	virtual void OnEvent(DWORD events);
protected:
	SOCKET mSock = -1;
};

//XiongWanPing 2016.01.25
//linux下采用epoll,ios下采用kqueue,来实现消息循环
//epoll和kqueue的用法太相似了，所以放在一起,没有必要弄个单独的Looper_Ios
class CORE_EXPORT Looper_Linux :public LooperImpl
{
	SUPER(LooperImpl)
	friend class SmartTlsLooperManager_Linux;

public:
	Looper_Linux();
	virtual ~Looper_Linux();

protected:
	void OnCreate();
	virtual int StartHelper(bool newThread);
	virtual void _StackLooperSendMessage(tagLoopMessageInternal& loopMsg);

	int CreateSocketPair();

	virtual int getMessage(tagLoopMessageInternal& msg);

	bool PostQueuedCompletionStatus(HANDLE handle, DWORD bytes = 0, ULONG_PTR key = 0, LPOVERLAPPED lpOverlapped = 0);
protected:
	std::shared_ptr<EpollProxyDummy> mSockPairSendProxy;
	std::shared_ptr<EpollProxyDummy> mSockPairReceiveProxy;

#ifdef _CONFIG_DEBUG_LOOPER
	ULONGLONG mTickCheckTimer = 0;
#endif
};
}
}