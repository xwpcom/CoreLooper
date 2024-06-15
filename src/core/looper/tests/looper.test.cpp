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
	TEST_METHOD(mainLooper)
	{
		class MainLooper :public MainLooper_
		{
			long mTimer_test = 0;
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimer_test, 100);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_test)
				{
					PostQuitMessage(0);
					return;
				}

				__super::OnTimer(id);
			}

		};

		make_shared<MainLooper>()->StartRun();

	}

	//创建一些looper,测试互发消息
	TEST_METHOD(sendMessage_)
	{
		static int gCount = 20;
		static int gValue = 0;
		class MainLooper :public MainLooper_
		{
			enum
			{
				BM_TEST = 1,
				BM_ADD,
			};

			void OnCreate()
			{
				__super::OnCreate();

				class WorkLooper :public Looper
				{
				public:
					void bindBuddy(shared_ptr<WorkLooper> prev, shared_ptr<WorkLooper> next)
					{
						mPrevLooper = prev;
						mNextLooper = next;
					}

				protected:
					LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
					{
						switch (msg)
						{
						case BM_TEST:
						{
							gValue++;

							auto id = GetId();

							if (id > 0)
							{
								auto obj = mPrevLooper.lock();
								if (obj)
								{
									obj->sendMessage(BM_ADD);
								}
							}

							auto obj = mNextLooper.lock();
							if (obj)
							{
								obj->sendMessage(BM_TEST);
							}
							return 0;
						}
						case BM_ADD:
						{
							gValue++;

							return 0;
						}
						}
						return __super::OnMessage(msg, wp, lp);
					}

					weak_ptr<WorkLooper> mPrevLooper;
					weak_ptr<WorkLooper> mNextLooper;
				};

				vector<shared_ptr<WorkLooper>> loopers;
				for (int i = 0; i < gCount; i++)
				{
					auto obj = make_shared<WorkLooper>();
					loopers.push_back(obj);

					obj->SetId(i);
					AddChild(obj);
					obj->Start();
				}
				
				for (int i = 0; i < gCount; i++)
				{
					if (i > 0 && i<gCount-1)
					{
						loopers[i]->bindBuddy(loopers[i-1], loopers[i+1]);
					}
					else if(i==0)
					{
						loopers[i]->bindBuddy(nullptr, loopers[i + 1]);
					}
					else if (i == gCount-1)
					{
						loopers[i]->bindBuddy(loopers[i - 1],nullptr);
					}
				}

				loopers[0]->sendMessage(BM_TEST);

				SetTimer(mTimeout,2000);
			}

			long mTimeout = 0;

			void OnTimer(long id)
			{
				if (id == mTimeout)
				{
					LogI(TAG, "gValue = %d",gValue);
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

		};

		make_shared<MainLooper>()->StartRun();

	}
};

}
