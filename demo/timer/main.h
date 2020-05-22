#pragma once

#include "base.inl"
using namespace Bear::Core;
class MainLooper :public Looper
{
	SUPER(Looper)
public:
	MainLooper();

protected:
	void OnCreate();
	void OnTimer(long id);

	long mTimerDelayExit = 0;
	long mTimerTest = 0;
};
