#include "stdafx.h"
#include "wndmanager.h"
#include "mfcmainlooper.h"
#include "uirunnable.h"

namespace Bear {
namespace Windows {
enum { WM_SELF_DELETE = WM_USER + 2 };

BEGIN_MESSAGE_MAP(WndManager, CWnd)
	ON_WM_TIMER()
	ON_MESSAGE(WM_RUNNABLE_EVENT, OnRunnableEvent)
	ON_MESSAGE(WM_SELF_DELETE, OnSelfDelete)
END_MESSAGE_MAP()

LRESULT WndManager::OnSelfDelete(WPARAM, LPARAM)
{
	// safe: run on the window's thread and after other messages are processed
	DestroyWindow(); // ensure window destroyed
	//delete this;     // safe to delete on owning thread
	return 0;
}

void WndManager::DestroyInstance()
{
	if (gInstance)
	{
		/*
		HWND hwnd = gInstance->GetSafeHwnd();
		// ask the window to delete itself on its own message thread
		if (hwnd)
			::PostMessage(hwnd, WM_SELF_DELETE, 0, 0);
		else
			delete gInstance;
		gInstance = nullptr;
		*/
	}
}

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

WndManager::~WndManager()
{

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
		if (info->fn)
		{
			info->fn();
		}
		else if (info->fn2)
		{
			info->fn2(info->body);
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	info->mSelfRef = nullptr;
	return 0;
}

}
}
