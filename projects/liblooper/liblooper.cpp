#include "pch.h"
#include "CppUnitTest.h"
#include "looper.h"
#include "timer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Core;

namespace liblooper
{
	TEST_CLASS(looper_)
	{
	public:
		
		TEST_METHOD(baseRun)
		{
			string mTag = "baseRun";

			uint64_t minDelay = 0x80000000ULL + 5;
			if (minDelay & 0x80000000)
			{
				minDelay = min(minDelay, 0x7FFFFFFFULL);
			}

			int ms = (int)minDelay;
			auto obj = make_shared<Looper>();

			weak_ptr<Looper> weak_self = obj;
			auto _timer = std::make_shared<Timer>(2.0f, [mTag,weak_self]() {
				auto strong_self = weak_self.lock();
				if (!strong_self) {
					return false;
				}
				
				logV(mTag)<< "timer";
				//strong_self->onManager();
				return true;
											 }, obj);

			obj->runLoop();
		}
	};
}
