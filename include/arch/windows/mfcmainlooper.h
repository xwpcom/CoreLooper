#pragma once
#include "uirunnable.h"
#include <memory>
namespace Bear {
namespace Windows {
using namespace Bear::Core;
using namespace std;

enum
{
	WM_LOOPER_EVENT = WM_USER + 1,
};

//XiongWanPing 2019.10.01

class CORE_EXPORT MfcMainLooper :public MainLooper_
{
	friend class WndManager;
public:
	MfcMainLooper();
	virtual ~MfcMainLooper();
	void postRunnableUI(const std::function<void()>& fn,weak_ptr<UIProxy>&,const string& body="");
	void postRunnableUI(const std::function<void(const string&)>& fn, weak_ptr<UIProxy>&, const string& body = "");
protected:
	void SetFireEventSink(HWND wnd, UINT msg)
	{
		ASSERT(!mHwnd);

		mHwnd = wnd;
		mHwndMessage = msg;
	}
	static shared_ptr<MfcMainLooper> CheckCreateMainLooper();

	HWND mHwnd = nullptr;
	UINT mHwndMessage = 0;
};

}
}