#pragma once

namespace Bear {
namespace Windows {
using namespace Bear::Core;

//XiongWanPing 2019.10.01
/* 用法见BearStudio */

class CORE_EXPORT WndManager :public CWnd
{
public:
	static WndManager* Instance();
	void DestroyInstance();

	void NotifyQuit();
	bool CanQuitNow()const;

protected:
	WndManager();
	void Create();

	DECLARE_MESSAGE_MAP()

	enum
	{
		WM_RUNNABLE_EVENT = (WM_USER + 1),
	};
	LRESULT OnRunnableEvent(WPARAM wp, LPARAM lp);

	static WndManager*	gInstance;
	weak_ptr<Handler>	mLooper;
	bool mHasNotifyQuit = false;
};

}
}