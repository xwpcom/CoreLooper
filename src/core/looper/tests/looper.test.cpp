#include "stdafx.h"
#include "CppUnitTest.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "net/dnslooper.h"

class Demo
{
public:
	void* operator new(size_t) = delete;
};


#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

/*
XiongWanPing 2016~
本单元测试用来测试各种场景，包括正常场景和故意构造的极端调用场景,
以此来保证CoreLooper框架在各种调用场景的稳定性
.每个测试必须能快速进行，以便快速迭代
*/
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;
static const char* TAG = "Core";

namespace UnitTest
{
TEST_CLASS(Looper_)
{
public:
	TEST_METHOD(dnsLooper)
	{
		class MainLooper :public MainLooper_
		{
			enum
			{
				BM_DNS_ACK=1,
			};

			long mTimer_parseDns = 0;
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimer_parseDns, 1);
				DelayExit(5000);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_parseDns)
				{
					auto dnsLooper = _MObject(DnsLooper, "DnsLooper");
					if (!dnsLooper)
					{
						auto looper = make_shared<DnsLooper>();
						AddChild(looper);
						looper->Start();

						dnsLooper = _MObject(DnsLooper, "DnsLooper");
					}

					if (dnsLooper)
					{
						string addr = "163.com";
						dnsLooper->AddRequest(addr, shared_from_this(), BM_DNS_ACK);
					}
					return;
				}

				__super::OnTimer(id);
			}

			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				switch (msg)
				{
				case BM_DNS_ACK:
				{
					string dns = (const char*)wp;
					string ip = (const char*)lp;
					LogV(TAG,"%s=[%s]", dns.c_str(), ip.c_str());
					return 0;
				}
				}
				return __super::OnMessage(msg, wp, lp);
			}

		};

		make_shared<MainLooper>()->StartRun();

	}
};

}
