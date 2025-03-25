#include "pch.h"
#include "CppUnitTest.h"
#include "looper.h"
#include "timer.h"
#include "iniConfiger.h"
#include "system.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Core;

namespace liblooper
{
	TEST_CLASS(looper_)
	{
	public:
		TEST_METHOD(zl_base)
		{
			string folder = appFolder();
			auto filePath = folder + "/zlStudy.ini";

			IniConfiger cfg;
			cfg.parseFile(filePath);
			int channel = cfg["lora.channel"];
			string channelText = cfg["lora.channel"];
			int x = 0;
			cfg["lora.key"] = "123";
			cfg["lora.sn"] = 3;
			cfg["s1.s2.sn"] = 3;
			cfg.dumpFile(folder + "/zlStudy.save.ini");
		}

		TEST_METHOD(baseRun)
		{
			class MainLooper :public Looper
			{
			public:
				MainLooper()
				{
				}

				void onCreate()
				{
					__super::onCreate();

					weak_ptr<Looper> weak_self = shared_from_this();
					mTimer_test = make_shared<Timer>(2000, [this, weak_self]() {
						auto strong_self = weak_self.lock();
						if (!strong_self) {
							return false;
						}

						++mIndex;
						logV(mTag) << "timer,index="<< mIndex;
						//strong_self->onManager();

						if (mIndex == 2)
						{
							mTimer_ping = make_shared<Timer>(1000, [this, weak_self]() {
								auto strong_self = weak_self.lock();
								if (!strong_self) {
									return false;
								}

								if (mTimer_test)
								{
									logV(mTag) << "kill test timer";
									mTimer_test = nullptr;
								}
								else
								{
									logV(mTag) << "kill ping timer";
									mTimer_ping = nullptr;
								}

								return true;
							});
						}

						return true;
					});
				}
			protected:
				Timer::Ptr mTimer_test;
				Timer::Ptr mTimer_ping;
				string mTag = "baseRun";
				int mIndex = 0;

			};

			auto obj = make_shared<MainLooper>();
			obj->onCreate();
			obj->runLoop();
		}
	};
}
