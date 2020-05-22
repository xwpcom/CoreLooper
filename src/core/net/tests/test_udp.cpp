#include "stdafx.h"
#include "CppUnitTest.h"
#include "net/baseudpserver.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::Net;

TEST_CLASS(Udp)
{
	class MainLooper :public Looper
	{
		SUPER(Looper);
		enum
		{
			eTimerExit,
		};
	public:

	protected:
		int InitInstance()
		{
			auto obj = make_shared<BaseUdpServer>();
			obj->StartServer(8000);

			int seconds = 10;
			SetTimer(eTimerExit, seconds * 1000);
			return __super::InitInstance();
		}

		void OnTimer(long id)
		{
			switch (id)
			{
			case eTimerExit:
			{
				PostQuitMessage();
				return;
			}
			}

			__super::OnTimer(id);
		}

	};
	TEST_METHOD(Test)
	{
		auto obj = make_shared<MainLooper>();
		obj->StartRun();

	}
};
