#include "stdafx.h"
#include "arch/android/jnimainlooper.h"
#include "arch/android/jnilooper.h"

namespace Bear {
namespace Android {
static const char* TAG = "JniMainLooper";

shared_ptr<Handler> JniMainLooper::gInstance;
JniMainLooper::JniMainLooper()
{
}

void JniMainLooper::OnCreate()
{
	__super::OnCreate();

	{
		auto obj = make_shared<JniLooper>();
		obj->Start();
		AddChild(obj);
	}
}

void JniMainLooper::CheckCreateJniMainLooper()
{
	auto obj = Looper::GetMainLooper();
	if (obj)
	{
		auto looper = dynamic_pointer_cast<JniMainLooper>(obj->shared_from_this());
		if (!looper)
		{
			LogE(TAG,"please make sure MainLooper is kind of JniMainLooper");
		}
	}
	else
	{
		auto obj = make_shared<JniMainLooper>();
		gInstance = obj;//make self alive forever
		obj->Start();
		obj->sendMessage(BM_NULL);
	}

}

}
}
