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
