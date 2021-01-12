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
	TEST_METHOD(IoIdleTest)
	{
		class MainLooper :public MainLooper_
		{
			long mTimer_Test=0;
			void OnCreate()
			{
				__super::OnCreate();


				SetTimer(mTimer_Test, 3000);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_Test)
				{
					static int idx = -1;
					++idx;
					LogV(TAG, "timer,idx=%04d",idx);
				}

				__super::OnTimer(id);
			}
		};
	
		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TcpListener_)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				{
					LogV(TAG,"main threadId=%d", ShellTool::GetCurrentThreadId());

					
					for(int i=0;i<5;i++)
					{
						auto obj = make_shared<TcpListener_Windows>();
						AddChild(obj);
						obj->StartListener(80+i);

						obj->SignalAccept.connect(this, &MainLooper::OnAccept);
					}

					DelayExit(5000);
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



