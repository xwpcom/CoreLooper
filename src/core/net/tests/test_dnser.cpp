#include "stdafx.h"
#include "CppUnitTest.h"
#include "net/dnser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::Net;

namespace Dns_UnitTest
{
TEST_CLASS(_Dnser)
{
	class MainLooper :public MainLooper_
	{
		SUPER(MainLooper_);
	public:

	protected:
		void OnCreate()
		{
			__super::OnCreate();

			auto dns = make_shared<Dnser>();
			AddChild(dns);

		}

	};
};

}

