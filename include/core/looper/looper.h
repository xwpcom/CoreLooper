#pragma once

#ifdef _MSC_VER
#include "../../arch/windows/looper_windows.h"
#define BASE_LOOPER Looper_Windows
#else
//android,ios and linux
#include "arch/linux/looper_linux.h"
#define BASE_LOOPER Looper_Linux
#include "arch/linux/linuxsignal.h"
#endif

namespace Bear {
namespace Core
{
class LooperImpl;
//XiongWanPing 2016.01.26
//Looper用来给Android,ios,linux和windows c++代码提供统一的跨线程通讯消息机制
//它参考了Android的Looper,Message和Handler构架
//用户自定义looper应该从Looper或其子类继承
class CORE_EXPORT Looper : public BASE_LOOPER
{
public:
	virtual ~Looper();
	static Looper *GetMainLooper();
	static int SetMainLooper(Looper*);
	static bool IsMainLooper(LooperImpl *looper);
};

class DelayExitRunnable :public Runnable
{
	void Run()
	{
		Looper::GetMainLooper()->PostQuitMessage();
	}
};

//main looper helper
class CORE_EXPORT MainLooper_ :public Looper
{
	SUPER(Looper);
public:
	MainLooper_()
	{
		auto name = "MainLooper";
		SetObjectName(name);
		mThreadName = name;
		SetMainLooper(this);
	}

	void DelayExit(int ms=0)
	{
		postDelayedRunnable(make_shared<DelayExitRunnable>(), ms);
	}
protected:
	void OnCreate()
	{
		__super::OnCreate();

#ifndef _MSC_VER
		/* I hate signal very much */
		{
			LinuxSignal t;
			//t.Init();
			t.InitEx();
		}

#endif
	}
};

}
}