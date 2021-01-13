#include "stdafx.h"
#include "dnslooper.h"
#ifdef _MSC_VER
#pragma comment(lib,"Iphlpapi.lib")
#include <Iptypes.h>
#include <Iphlpapi.h>
#include <IPExport.h>
#include <ws2ipdef.h>
#include <Ws2tcpip.h>
#else

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <memory>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>

#ifndef __APPLE__
#include <linux/if.h>
#endif


//#include <net/if.h>
extern "C"
{
	char *if_indextoname(unsigned ifindex, char *ifname);
};

#endif

#undef bzero
#define bzero(a,size) memset(a,0,size)

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {

enum
{
	BM_ADD_REQUEST,
	BM_CANCEL_REQUEST,
};

enum
{
	eTimerCheck,
};

bool DnsLooper::mThreadRunning = false;
char DnsLooper::mDns[128];
char DnsLooper::mIP[16];

static const char* TAG = "DnsLooper";

#ifdef _MSC_VER
#define _TLS	__declspec(thread)
#else
#define _TLS	__thread
#endif

/*
static _TLS bool mThreadRunning = false;
static _TLS char mDns[128];
static _TLS char mIP[16];
*/

#ifdef _DEBUG
//用来保证:同一app中只允许有一个DnsLooper实例
DnsLooper *gDnsLooper = nullptr;
#endif

DnsLooper::DnsLooper()
{
#ifdef _DEBUG
	ASSERT(!gDnsLooper);
	gDnsLooper = this;
#endif

	SetObjectName("DnsLooper");
	mThreadName = GetObjectName();
}

DnsLooper::~DnsLooper()
{
#ifdef _DEBUG
	gDnsLooper = nullptr;
#endif
}

int DnsLooper::AddRequest(string dns, weak_ptr<Handler> handler, UINT msg)
{
	tagItemInfo info;
	info.mDns = dns;
	info.mHandler = handler;
	info.mMsg = msg;

	int ret = (int)sendMessage(BM_ADD_REQUEST, (WPARAM)&info);
	if (ret)
	{
		LogW(TAG,"fail BM_ADD_REQUEST,dns=[%s]", dns.c_str());
		ASSERT(FALSE);
	}
	return ret;
}

void DnsLooper::CancelRequest(weak_ptr<Handler> handler)
{
	tagItemInfo info;
	info.mHandler = handler;

	sendMessage(BM_CANCEL_REQUEST, (WPARAM)&info);
}

LRESULT DnsLooper::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_ADD_REQUEST:						  
	{
		tagItemInfo& info = *(tagItemInfo*)wp;
		mItems.push_back(info);

		if (!mThreadRunning)
		{
			StartDnsThread();
		}

		return 0;
	}

	case BM_CANCEL_REQUEST:
	{
		tagItemInfo& info = *(tagItemInfo*)wp;
		shared_ptr<Handler> handler = info.mHandler.lock();
		if (!handler)
		{
			return -1;
		}

		for (auto iter = mItems.begin(); iter != mItems.end();)
		{
			auto iterSave = iter;
			++iter;

			auto handler2 = iterSave->mHandler.lock();
			if (handler2 && handler2.get() == handler.get())
			{
				mItems.erase(iterSave);
			}
		}

		return 0;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

int DnsLooper::StartDnsThread()
{
	if (mThreadRunning)
	{
		return 0;
	}

	mThreadRunning = true;

	bzero(mDns, sizeof(mDns));
	strncpy(mDns, mItems.front().mDns.c_str(), sizeof(mDns) - 1);
	mIP[0] = 0;

	ShellTool::QueueUserWorkItem((LPTHREAD_START_ROUTINE)_DnsThreadCB, NULL);
	SetTimer(mTimerCheck,10);//定时检查mThreadRunning,看是否解析完成
	return 0;
}

//解析mDns,把结果放入mIP
void* DnsLooper::_DnsThreadCB(void *p)
{
	UNUSED(p);

#ifdef _DEBUG
	//ShellTool::Sleep(3 * 1000);//模拟网络较慢时的场景
#endif
	ShellTool::SetThreadName("_DnsThreadCB");
	{
		if (strcmp(mDns, "localhost") == 0)
		{
			strncpy(mIP, "127.0.0.1", sizeof(mIP) - 1);
		}
		else
		{
			DWORD dwRetval;
			struct addrinfo *result = NULL;
			struct addrinfo *ptr = NULL;
			struct addrinfo hints;

			bzero(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			//LogV(TAG,"getaddrinfo(%s)#begin", mDns);
			dwRetval = getaddrinfo(mDns, nullptr, &hints, &result);//注意在断网情况下，此api可能阻塞60秒或更长时间
			//LogV(TAG,"getaddrinfo(%s)#end,ret=%d", mDns, dwRetval);
			if (dwRetval == 0)
			{
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
				{
					sockaddr_in *sa = (sockaddr_in *)ptr->ai_addr;
					//DT("name: %s ip:%s", ptr->ai_canonname,
					  //    inet_ntop(AF_INET, &sa->sin_addr.s_addr, ip, sizeof (ip)));  
					const char *ipThis = inet_ntoa(sa->sin_addr);
					//DT("Length of this sockaddr: %d,ip=[%s]", ptr->ai_addrlen,ip);
					//LogV(TAG,"Canonical name: %s,ipThis=%s", ptr->ai_canonname,ipThis);
					if (ipThis)
					{
						strncpy(mIP, ipThis, sizeof(mIP) - 1);
						break;
					}
				}
			}
			else
			{
				LogW(TAG,"getaddrinfo(%s) error=%d(%s)", mDns, errno, strerror(errno));
			}

			//DT("freeaddrinfo#begin,result=0x%08x", result);
			if (result)
			{
				//在android上面result为nullptr时会crash
				freeaddrinfo(result);
				result = nullptr;
			}
			//DT("freeaddrinfo#end,result=0x%08x", result);
		}
	}

	mThreadRunning = false;
	return nullptr;
}

void DnsLooper::OnTimer(long timerId)
{
	if(timerId == mTimerCheck)
	{
		if (!mThreadRunning)
		{
			KillTimer(mTimerCheck);

			OnDnsParseDone(mDns, mIP);

			if (!mItems.empty())
			{
				StartDnsThread();
			}
		}

		return;
	}

	__super::OnTimer(timerId);
}

void DnsLooper::OnDnsParseDone(string dns, string ip)
{
	ASSERT(IsMyselfThread() && !mThreadRunning);

	if (mItems.empty())
	{
		return;
	}

	tagItemInfo item = mItems.front();
	if (item.mDns == dns)
	{
		mItems.pop_front();

		if (item.mCancel)
		{
			LogV(TAG,"DnsLooper:dns %s to ip %s,but handler is cancelled", item.mDns.c_str(), ip.c_str());
		}
		else
		{
			shared_ptr<Handler> handler = item.mHandler.lock();
			if (handler)
			{
				if (!ip.empty())
				{
					//LogV(TAG,"DnsLooper:dns [%s] to ip [%s]", item.mDns.c_str(), ip.c_str());
				}

				handler->sendMessage(item.mMsg, (WPARAM)item.mDns.c_str(), (LPARAM)ip.c_str());
			}
		}
	}
}

}
}
}