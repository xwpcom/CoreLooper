#include "stdafx.h"
#include "looper_linux.h"
#include "looper/looper.h"
#include "../../core/looper/timermanager.h"
#include "../../core/looper/handlerinternaldata.h"
#include "../../core/looper/looperinternaldata.h"

#ifdef _MSC_VER
#include <process.h>
#endif

using namespace std;

namespace Bear {
namespace Core
{
using namespace Net;

static const char* TAG = "Looper_Linux";

class EpollProxyDummy :public EpollProxy
{
public:
	EpollProxyDummy(SOCKET s) :EpollProxy(s)
	{
	}

	~EpollProxyDummy()
	{
		SockTool::CLOSE_SOCKET(mSock);
	}
	SOCKET GetSock()const
	{
		return mSock;
	}

	virtual void OnEvent(DWORD events)
	{
		if (events & EPOLLIN)
		{
			char buf[1024];
			recv(mSock, buf, sizeof(buf), 0);//data is useless,just trigger epoll_wait returns
		}
		else
		{
			LogW(TAG,"OnEvent,events = 0x%x", events);
		}
	}
};

#if defined  _MSC_VER
long epoll_create(int)
{
	LogV(TAG,"epoll_create");
	return 1;
}
long epoll_ctl(int epfd, int op, int sock, epoll_event *) { return 0; }
long epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) { return 0; }
#else
#define __super LooperImpl
#endif

void EpollProxy::OnEvent(DWORD events)
{
	LogV("EpollProxy", "this=0x%08x,%s, sock=%d, events=0x%x", this, __func__, mSock, events);
}

Looper_Linux::Looper_Linux()
{
	SetObjectName("Looper_Linux");
	mThreadName = GetObjectName();
	//LogV(TAG,"%s,this=0x%x", __func__,this);
}

Looper_Linux::~Looper_Linux()
{
	//LogV(TAG,"%s", __func__);
}

void Looper_Linux::OnCreate()
{
	__super::OnCreate();

#ifdef _CONFIG_DEBUG_LOOPER
	mTickCheckTimer = ShellTool::GetTickCount64();
#endif
}

int Looper_Linux::StartHelper(bool newThread)
{
	//LogV(TAG,"%s", __func__);
	if (mLooperInternalData->mLooperRunning)
	{
		ASSERT(FALSE);
		return 0;
	}

	{
		ASSERT(mLooperHandle == INVALID_HANDLE_VALUE);
#ifdef __APPLE__
		mLooperHandle = (HANDLE)kqueue();
#else
		mLooperHandle = (HANDLE)(LONGLONG)epoll_create(1);
#endif
	}

	{
		mInternalData->mSelfRef = shared_from_this();
	}

	CreateSocketPair();

	if (!mLooperInternalData->mExitEvent && mLooperInternalData->mOwnerLooper.lock())
	{
		SetExitEvent(mLooperInternalData->mOwnerLooper.lock()->CreateExitEvent());
	}

	//ASSERT(mExitEvent);

	mLooperInternalData->mLooperRunning = true;
	mLooperInternalData->mBMQuit = false;

	if (newThread)
	{
		mInternalData->mCreated = true;
		BOOL ok = ShellTool::QueueUserWorkItem((LPTHREAD_START_ROUTINE)_WorkThreadCB, (LPVOID)this, 0);
		return ok ? 0 : -1;
	}

	ASSERT(CurrentLooper() == nullptr);
	LooperImpl *looperCache = CurrentLooper();
	mInternalData->mCreated = true;
	_WorkThreadCB(this);
	SetCurrentLooper(looperCache);
	return 0;
}

#define _CONFIG_SMART_TLS_LOOPER

#ifdef _CONFIG_SMART_TLS_LOOPER
class SmartTlsLooperManager_Linux
{
	static void clear();
public:
	shared_ptr<Looper_Linux> PopTlsLooper()
	{
		{
			AutoLock lock(&mCS);
			if (!mLoopers.empty())
			{
				auto obj = mLoopers.back();
				mLoopers.pop_back();
				return obj;
			}
		}

		{
			static bool first = true;
			if (first)
			{
				first = false;

				atexit(SmartTlsLooperManager_Linux::clear);
			}

		}

		auto looper = make_shared<Looper_Linux >();
		looper->mThreadId = ShellTool::GetCurrentThreadId();
		looper->mLooperInternalData->mAttachThread = true;

#ifdef __APPLE__
		looper->mLooperHandle = (HANDLE)kqueue();
#else	
		looper->mLooperHandle = (HANDLE)(LONGLONG)epoll_create(1);
#endif
		looper->CreateSocketPair();

		return looper;
	}

	void PushTlsLooper(shared_ptr<Looper_Linux> obj)
	{
		AutoLock lock(&mCS);
		mLoopers.push_back(obj);
	}

protected:
	CriticalSection mCS;
	list<shared_ptr<Looper_Linux>> mLoopers;
};

static SmartTlsLooperManager_Linux gTlsLooperManager;

void SmartTlsLooperManager_Linux::clear()
{
	AutoLock lock(&gTlsLooperManager.mCS);
	gTlsLooperManager.mLoopers.clear();
}

class SmartTlsLooper_Linux
{
public:
	SmartTlsLooper_Linux(LooperImpl* obj)
	{
		mLooper = gTlsLooperManager.PopTlsLooper();
		mLooper->mThreadId = ShellTool::GetCurrentThreadId();

		mLooperImpl = obj;
		mLooperImpl->SetCurrentLooper(mLooper.get());
	}

	~SmartTlsLooper_Linux()
	{
		gTlsLooperManager.PushTlsLooper(mLooper);
		mLooperImpl->SetCurrentLooper(nullptr);
	}

	LooperImpl* mLooperImpl;
	shared_ptr<Looper_Linux> mLooper;
};
#endif


void Looper_Linux::_StackLooperSendMessage(tagLoopMessageInternal& loopMsg)
{
#ifdef _CONFIG_SMART_TLS_LOOPER
	SmartTlsLooper_Linux obj(this);
	sendMessageHelper(loopMsg, *obj.mLooper.get());
#else

	//构造一个stack looper来实现sendMessage
	auto looper = make_shared<Looper_Linux>();
	SetCurrentLooper(looper.get());
	looper->mThreadId = ShellTool::GetCurrentThreadId();
	looper->mLooperInternalData->mAttachThread = true;
#ifdef __APPLE__
	looper->mLooperHandle = (HANDLE)kqueue();
#else	
	looper->mLooperHandle = (HANDLE)(LONGLONG)epoll_create(1);
#endif
	looper->CreateSocketPair();
	sendMessageHelper(loopMsg, *looper.get());
	SetCurrentLooper(nullptr);
	looper = nullptr;
#endif

}

int Looper_Linux::getMessage(tagLoopMessageInternal& msg)
{
	mLooperTick = ShellTool::GetTickCount64();

	do
	{
		{
			AutoLock lock(&mLooperInternalData->mMessageLock);
			auto & msgList = mLooperInternalData->mMessageList;
			if (!msgList.empty())
			{
				msg = msgList.front();
				msgList.pop_front();

				mLastIoTick = mLooperTick;

#ifdef _CONFIG_DEBUG_LOOPER
				/*
				if (isMainLooper)
				{
					string name;
					if (msg.mHandler)
					{
						name = msg.mHandler->GetName();
					}

					//LogV(TAG,"mMessageList.pop_front %s.msg=%d", name.c_str(), msg.mMsg);

					//消息太多，导致一直没有机会处理timer?
					auto tickNow = ShellTool::GetTickCount64();
					auto tick = tickNow - mTickCheckTimer;
					if (tick > 300)
					{
						LogW(TAG,"%s no timer for " FMT_LONGLONG " ms,tickNow=" FMT_LONGLONG ",mTickCheckTimer=" FMT_LONGLONG
							, mThreadName.c_str()
							, tick
							, tickNow
							, mTickCheckTimer
						);

						if (tick > 5000)
						{
							int x = 0;
						}
					}
				}
				//*/
#endif
				return 0;
			}
		}

		if (mLooperInternalData->mBMQuit && CanQuitLooperNow())
		{
			return -1;
		}

		DWORD cmsDelayNext = INFINITE;
#ifdef _CONFIG_DEBUG_LOOPER
		mTickCheckTimer = ShellTool::GetTickCount64();
#endif
		ProcessTimer(cmsDelayNext, mLooperTick-mLastIoTick);
#ifdef _CONFIG_DEBUG_LOOPER
		if (isMainLooper)
		{
			auto tick = ShellTool::GetTickCount64() - mTickCheckTimer;
			if (tick > 50)
			{
				mTickCheckTimer = ShellTool::GetTickCount64();

				static int idx = -1;
				LogW(TAG,"ProcessTimer[%04d].tick=" FMT_LONGLONG, ++idx, tick);
			}
		}
#endif

		{
			//static int idx = -1;
			//++idx;
			//LogV(TAG,"%s,idx=%04d",__func__,idx);
		}

		DWORD minMS = 1;
		if (cmsDelayNext < minMS)
		{
			cmsDelayNext = minMS;
		}

		
		{
			ASSERT(cmsDelayNext);//如果为0，会形成busy loop,占用大量cpu
			ASSERT(mLooperHandle != INVALID_HANDLE_VALUE);

#ifdef __APPLE__
			DWORD waitms = cmsDelayNext;
			//XiongWanPing 2017.03.09
			//发现waitms比较小时,在ipad上面会占用较高的cpu,所以适当调大
			//minms=20,cpu=0%
			//minms=10,cpu=1%
			//minms=5,cpu=%2
			//*
			//const int minms=10;
			//if(waitms<minms)
			{
				//waitms=minms;
			}
			//*/

			struct timespec timeout;
			timeout.tv_sec = waitms / 1000;
			timeout.tv_nsec = (waitms % 1000) * 1000 * 1000;

			const int kMaxEvents = 64;
			struct kevent arr[kMaxEvents];
			int n = kevent((int)mLooperHandle, NULL, 0, arr, COUNT_OF(arr), &timeout);
			if (n > 0)
			{
				mLastIoTick = mLooperTick;
			}
			for (int i = 0; i < n; i++)
			{
				EpollProxy *handler = (EpollProxy*)(intptr_t)arr[i].udata;
				int events = arr[i].filter;
				if (!handler)
				{
					//在xcode中调试时,暂停的时间稍长,再运行时就会收到几个handler为nullptr的信号,
					LogV(TAG,"handler is nullptr,events=%d", events);
					continue;
				}

				DWORD mask = 0;
				if (arr[i].flags & EV_EOF)
				{
					mask |= EPOLLERR;
				}

				//kqueue的事件不会重叠
				//#define EVFILT_READ		(-1)
				//#define EVFILT_WRITE		(-2)

				if (events == EVFILT_READ)
				{
					mask |= EPOLLIN;
				}
				else if (events == EVFILT_WRITE)
				{
					mask |= EPOLLOUT;
				}
				else
				{
					LogW(TAG,"unknown event=0x%04x\r\n", events);
					mask |= EPOLLERR;
				}

				handler->OnEvent(mask);
			}
#else
			epoll_event events[1024];
			int ret = epoll_wait((int)(LONGLONG)mLooperHandle, events, COUNT_OF(events), cmsDelayNext);
			if (ret == -1)
			{
				LogW(TAG, "epoll_wait error=%d(%s)",errno,strerror(errno));
			}

			if (ret > 0)
			{
				mLastIoTick = mLooperTick;
			}

			for (int i = 0; i < ret; i++)
			{
				if (events[i].data.ptr)
				{
					EpollProxy *proxy = (EpollProxy *)events[i].data.ptr;
					auto evt = events[i].events;
#ifdef _CONFIG_DEBUG_LOOPER
					auto tick = ShellTool::GetTickCount64();
#endif
					proxy->OnEvent(evt);
#ifdef _CONFIG_DEBUG_LOOPER
					tick = ShellTool::GetTickCount64() - tick;
					if (tick > 100)
					{
						static int idx = -1;
						++idx;
						LogW(TAG,"proxy.%p(evt=%d).tick=" FMT_LONGLONG, proxy, evt, tick);
					}
#endif
				}
				else
				{
					LogW(TAG,"evt.data.ptr is nullptr");
				}
			}
#endif

			if (mLooperInternalData->mAttachThread)
			{
				/* stack looper只触发，不收真正有用的消息 */
				return -1;
			}
		}
	} while (1);

	return -1;
}

bool Looper_Linux::PostQueuedCompletionStatus(HANDLE handle, DWORD bytes, ULONG_PTR key, LPOVERLAPPED lpOverlapped)
{
	UNUSED(handle);
	UNUSED(bytes);
	UNUSED(key);
	UNUSED(lpOverlapped);

#ifdef _MSC_VER
	ASSERT(bytes == 0);
	ASSERT(key == 0);
	ASSERT(lpOverlapped == nullptr);
#endif

	char buf[1] = { 'S' };
	SOCKET s = mSockPairSendProxy->GetSock();
	auto ret = send(s, buf, sizeof(buf), 0);
	if (ret != sizeof(buf))
	{
		LogW(TAG,"[%s]fail send,s=%d,error=%s(%d)",GetObjectName().c_str() , s, strerror(errno), errno);
		return false;
	}
	return true;
}

int Looper_Linux::CreateSocketPair()
{
	//LogV(TAG,"%s",__func__);
	ASSERT(!mSockPairSendProxy);
	ASSERT(!mSockPairReceiveProxy);

	SOCKET sockSend = INVALID_SOCKET;
	SOCKET sockReceive = INVALID_SOCKET;
	int ret = SockTool::socketpair(sockSend, sockReceive);
	if (ret)
	{
		LogW(TAG,"fail create socket pair");
		return -1;
	}

	if (ret == 0)
	{
#ifdef __APPLE__
		{
			SOCKET& s = sockSend;
			auto proxy=make_shared<EpollProxyDummy>(s);
			mSockPairSendProxy = proxy;
			struct kevent evt;
			EV_SET(&evt, s, EVFILT_READ, EV_ADD, 0, 0, (void*)proxy.get());
			ret = kevent(mLooperHandle, &evt, 1, NULL, 0, NULL);
			if (ret)
			{
				LogW(TAG,"error=%d,%s", errno, strerror(errno));
			}
		}
		{
			SOCKET& s = sockReceive;
			auto proxy = make_shared<EpollProxyDummy>(s);
			mSockPairReceiveProxy = proxy;
			struct kevent evt;
			EV_SET(&evt, s, EVFILT_READ, EV_ADD, 0, 0, (void*)proxy.get());
			ret = kevent(mLooperHandle, &evt, 1, NULL, 0, NULL);
			if (ret)
			{
				LogW(TAG,"error=%d,%s", errno, strerror(errno));
			}
		}
#else		
		struct epoll_event evt = { 0 };
		evt.events = EPOLLIN
			//| EPOLLOUT	//如果加上会一直触发此事件
			| EPOLLRDHUP
			| EPOLLERR
			;
		{
			SOCKET& s = sockSend;
			{
				auto proxy = make_shared<EpollProxyDummy>(s);
				mSockPairSendProxy = proxy;
				evt.data.ptr = (EpollProxy*)proxy.get();
			}
			ret = epoll_ctl((int)(LONGLONG)mLooperHandle, EPOLL_CTL_ADD, (int)(SOCKET)s, &evt);
			if (ret)
			{
				LogW(TAG,"error=%d,%s", errno, strerror(errno));
			}
		}

		{
			SOCKET& s = sockReceive;
			{
				auto proxy = make_shared<EpollProxyDummy>(s);
				mSockPairReceiveProxy = proxy;
				evt.data.ptr = (EpollProxy*)proxy.get();
			}
			ret = epoll_ctl((int)(LONGLONG)mLooperHandle, EPOLL_CTL_ADD, (int)(SOCKET)s, &evt);
			if (ret)
			{
				LogW(TAG,"error=%d,%s", errno, strerror(errno));
			}
		}
#endif
	}

	return 0;
}
}
}
