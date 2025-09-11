#pragma once
#include "uirunnable.h"
namespace Bear {
namespace Windows {
using namespace Bear::Core;

//XiongWanPing 2019.10.01
//运行在MfcMainLooper中

class CORE_EXPORT WndHandler:public Handler
{
public:
	WndHandler();
	void BindUIProxy(weak_ptr<UIProxy> obj)
	{
		mUIProxy = obj;
	}

	void uiPostRunnable(const std::function<void()>& fn);
	void uiPostRunnable(const std::function<void(const string&)>& fn);
	void uiPostRunnable(const std::function<void(const string&)>& fn,const string& body);

	weak_ptr<UIProxy> mUIProxy;
};

}
}
