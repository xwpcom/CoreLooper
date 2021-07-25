#include "stdafx.h"
#include "wndmanager.h"
#include "mfcmainlooper.h"
#include "uirunnable.h"

namespace Bear {
namespace Windows {

BEGIN_MESSAGE_MAP(WndManager, CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_RUNNABLE_EVENT, OnRunnableEvent)
END_MESSAGE_MAP()

WndManager* WndManager::Instance()
{
	if (gInstance)
	{
		return gInstance;
	}

	auto obj = new WndManager();
	obj->Create();

	{
		auto looper=MfcMainLooper::CheckCreateMainLooper();
		looper->SetFireEventSink(obj->GetSafeHwnd(), WM_LOOPER_EVENT);
		obj->mLooper = looper;
	}
	
	gInstance = obj;
	return obj;
}

WndManager* WndManager::gInstance;

WndManager::WndManager()
{
	gInstance = nullptr;
}

void WndManager::Create()
{
	if (GetSafeHwnd())
	{
		return;
	}

	auto name = _T("WndManager");
	{
		static BOOL bRegistered = FALSE;
		if (!bRegistered)
		{
			WNDCLASS wc = { 0 };
			wc.style = NULL;
			wc.lpfnWndProc = ::DefWindowProc;
			wc.hInstance = AfxGetInstanceHandle();
			wc.lpszClassName = name;
			wc.hbrBackground = NULL;
			wc.hCursor = NULL;
			bRegistered = AfxRegisterClass(&wc);
		}
	}

	auto ok = CreateEx(NULL, name, name, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0);
}

void WndManager::DestroyInstance()
{
	if (gInstance)
	{
		gInstance->DestroyWindow();
		gInstance = nullptr;
	}
}

void WndManager::NotifyQuit()
{
	if (mHasNotifyQuit)
	{
		return;
	}

	mHasNotifyQuit = true;
	auto obj = dynamic_pointer_cast<Looper>(mLooper.lock());
	if (obj)
	{
		//obj->postMessage(BM_DUMP_ALL);
		obj->PostQuitMessage();
	}
}

bool WndManager::CanQuitNow()const
{
	auto obj = mLooper.lock();
	auto ok = obj ? false : true;
	if (ok)
	{
		/* 处理looper可能已投递的未决WM_LOOPER_EVENT消息 */
		ShellTool::EatUpMessage();
	}

	return ok;
}

LRESULT WndManager::OnRunnableEvent(WPARAM wp, LPARAM lp)
{
	auto info = (tagUIRunnable*)wp;
	auto proxy = info->mProxy.lock();
	if (proxy)
	{
		info->fn();
	}

	info->mSelfRef = nullptr;
	return 0;
}

}
}
