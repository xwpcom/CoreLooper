#include "stdafx.h"
#ifdef _MSC_VER
#include "CppUnitTest.h"
#include "core/base/base.inl"
#include "speedserver.h"

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Test;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace Speed_Test {

TEST_CLASS(Speed_)
{
public:
	TEST_METHOD(Speed_2G)
	{
		//计算gprs传大文件的时长
		int bytesPerSecond = 2 * 1024;// 115200;
		int bytes = 1024 * 1024;
		int seconds = bytes / bytesPerSecond;
		DV("seconds=%d", seconds);

	}
	TEST_METHOD(TestSpeed)
	{
		class MainLooper :public Looper
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<SpeedServer>();
				AddChild(obj);
				obj->StartServer(1234);
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
	}
};

}

#endif
