#include "stdafx.h"
#include "CppUnitTest.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "tcplistener_windows.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;
static const char* TAG = "win";

namespace Core_Windows_UnitTest
{
TEST_CLASS(Core_Windows)
{
public:
	TEST_METHOD(TcpListener_)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				{
					DV("main threadId=%d", ShellTool::GetCurrentThreadId());

					
					for(int i=0;i<5;i++)
					{
						auto obj = make_shared<TcpListener_Windows>();
						AddChild(obj);
						obj->StartListener(80+i);

						obj->SignalAccept.connect(this, &MainLooper::OnAccept);
					}

					postDelayedRunnable(make_shared<DelayExitRunnable>(), 5 * 1000);
				}
			}

			void OnAccept(Handler*, SOCKET s)
			{
				int x = 0;
			}

		};

		make_shared<MainLooper>()->StartRun();
	}

};
}



