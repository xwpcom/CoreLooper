#include "stdafx.h"
#include "looper/looper.h"
#include "looper/looperimpl.h"
#include "looperinternaldata.h"

using namespace std;

namespace Bear {
namespace Core
{
static Looper *gMainLooper = nullptr;

Looper::~Looper()
{
	if (GetMainLooper() == this)
	{
		SetMainLooper(nullptr);
	}

	DV("%s,name=%s", __func__, GetObjectName().c_str());

	if (CurrentLooper())
	{
		if (CurrentLooper() == this)
		{
			//DV("%s is current looper,reset it", GetObjectName().c_str());
			SetCurrentLooper(nullptr);
		}
		else
		{
			if (mLooperInternalData->mAttachThread)
			{
				DW("cross thread destroy TLS looper,maybe left obsolete raw pointer,it is very dangerous!");
			}

			DW("%s is NOT current looper(%p)", GetObjectName().c_str(), CurrentLooper());
		}
	}
}

Looper *Looper::GetMainLooper()
{
	return gMainLooper;
}

bool Looper::IsMainLooper(LooperImpl *looper)
{
	return gMainLooper == looper;
}

int Looper::SetMainLooper(Looper* looper)
{
	if (looper)
	{
		DV("%s,name=%s", __func__, looper->mThreadName.c_str());

		if (gMainLooper)
		{
			ASSERT(FALSE);
			return -1;
		}

	}

	gMainLooper = looper;
	return 0;
}

//XiongWanPing 2016.06.09
//在非looper线程中如果要频繁向looper发送消息，每次都会创建临时looper
//为提高性能，可在非looper线程中调用本接口来创建一个looper并缓存
//stackLooper自己没有消息循环，只能向looper sendMessage或postMessage
/*
shared_ptr<Looper> Looper::BindTLSLooper()
{
	if (CurrentLooper())
	{
		ASSERT(FALSE);
		return nullptr;
	}

	auto looper = make_shared<Looper>();
	SetCurrentLooper(looper.get());
	looper->mThreadId = ShellTool::GetCurrentThreadId();
	looper->mLooperInternalData->mAttachThread = true;

#ifdef _MSC_VER
	looper->mLooperHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
#else
#if defined __APPLE__
	looper->mLooperHandle = kqueue();
#else
	looper->mLooperHandle = (HANDLE)epoll_create(1);
#endif
	looper->CreateSocketPair();
#endif

	return looper;
}
*/

}
}
