#include "stdafx.h"
#include "mfcmainlooper.h"
#include "uirunnable.h"

namespace Bear {
namespace Windows {
MfcMainLooper::MfcMainLooper()
{
}

MfcMainLooper::~MfcMainLooper()
{
}

shared_ptr<MfcMainLooper> MfcMainLooper::CheckCreateMainLooper()
{
	auto obj = Looper::GetMainLooper();
	if (obj)
	{
		auto looper = dynamic_pointer_cast<MfcMainLooper>(obj->shared_from_this());
		if (!looper)
		{
			DE("please make sure MainLooper is kind of MfcMainLooper");
		}
		
		return looper;
	}
	else
	{
		auto obj = make_shared<MfcMainLooper>();
		obj->Start();
		return obj;
	}
}

void MfcMainLooper::postRunnableUI(const std::function<void()>& fn, weak_ptr<UIProxy>& proxy,const string& body)
{
	auto item = make_shared<tagUIRunnable>();
	item->mSelfRef = item;
	item->fn = fn;
	item->mProxy = proxy;
	item->body = body;

	auto ptr = item.get();
	auto ok = ::PostMessage(mHwnd, mHwndMessage, (WPARAM)ptr, 0);
	if (!ok)
	{
		ASSERT(false);

		item->mSelfRef = nullptr;
	}
}

void MfcMainLooper::postRunnableUI(const std::function<void(const string&)>& fn, weak_ptr<UIProxy>& proxy, const string& body)
{
	auto item = make_shared<tagUIRunnable>();
	item->mSelfRef = item;
	item->fn2 = fn;
	item->mProxy = proxy;
	item->body = body;

	auto ptr = item.get();
	auto ok = ::PostMessage(mHwnd, mHwndMessage, (WPARAM)ptr, 0);
	if (!ok)
	{
		ASSERT(false);

		item->mSelfRef = nullptr;
	}
}

}
}
