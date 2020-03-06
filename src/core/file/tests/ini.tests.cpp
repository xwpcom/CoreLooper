#include "stdafx.h"
#include "CppUnitTest.h"
#include "ini.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

using namespace std;

namespace Core
{
TEST_CLASS(Ini_)
{
public:
	TEST_METHOD(Base)
	{
		Ini ini;

		//重载[]后有个缺点:没法追踪字段修改,不方便做自动保存
		auto value = "20200305";
		ini["info"]["version"] = value;
		auto& sec = ini["info"]["version"];
		Assert::IsTrue(sec == value);
	}

	TEST_METHOD(CheckVersion)
	{
		Ini ini;

		auto filePath = "d:/test/t.ini";

		Ini::Load(ini, filePath);
		auto ver = ini.version();
		ini.Set("info", "version", "12");
		auto ver2 = ini.version();

		Assert::IsTrue(ver2 != ver);
	}

	TEST_METHOD(LoadSave)
	{
		auto filePath = "d:/test/t.ini";

		auto version = "1234";
		{
			Ini ini;
			Ini::Load(ini, filePath);
			ini.Set("info", "version", version);
			Ini::Save(ini, filePath);
		}

		{
			Ini ini;
			Ini::Load(ini, filePath);
			auto ver=ini.GetString("info", "version");

			Assert::IsTrue(version == ver);
		}

	}

};
}
