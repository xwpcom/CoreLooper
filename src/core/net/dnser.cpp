#include "stdafx.h"
#include "dnser.h"
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

Dnser::Dnser()
{

}

Dnser::~Dnser()
{

}

int Dnser::AsyncGetHostByName(const string& dns, std::weak_ptr<Handler> handler, std::function<void()> fn)
{
	tagItem item;
	item.mHandler = handler;
	item.mCB = fn;

	mItems.push_back(item);

	if (!mTimerTest)
	{
		SetTimer(mTimerTest, 1000);
	}

	return 0;
}

void Dnser::CancelAsyncRequest(std::weak_ptr<Handler> handler)
{
	
}

void Dnser::OnTimer(long id)
{
	if (id == mTimerTest)
	{
		if (mItems.empty())
		{
			KillTimer(mTimerTest);
		}
		else
		{
			auto obj = mItems.front();
			mItems.pop_front();

			auto handler = obj.mHandler.lock();
			if (handler)
			{
				obj.mCB();
			}
		}

		return;
	}

	__super::OnTimer(id);
}


}
}
}