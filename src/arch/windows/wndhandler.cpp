#include "stdafx.h"
#include "wndhandler.h"
#include "mfcmainlooper.h"
namespace Bear {
namespace Windows {

WndHandler::WndHandler()
{
	SetObjectName("WndHandler");
}

void WndHandler::uiPostRunnable(const std::function<void()>& fn)
{
	auto obj = dynamic_pointer_cast<MfcMainLooper>(Looper::GetMainLooper()->shared_from_this());

	if (obj)
	{
		obj->postRunnableUI(fn,mUIProxy);
	}
	else
	{
		ASSERT(FALSE);
	}
}

}
}
