#include "stdafx.h"
#include "CppUnitTest.h"
#include "tlshelper.h"
#include "demohttps.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::Net::Https;

namespace Bear {
namespace Core {
namespace Crypt {

class HikMainLooper:public Looper
{
	SUPER(Looper);
public:
	HikMainLooper();
	~HikMainLooper();

protected:
	void OnCreate();
};

HikMainLooper::HikMainLooper()
{
	SetMainLooper(this);
}

HikMainLooper::~HikMainLooper()
{
}

void HikMainLooper::OnCreate()
{
	__super::OnCreate();
	//PostQuitMessage();

	AddChild(make_shared<DemoHttps>());
}


}
}
}

