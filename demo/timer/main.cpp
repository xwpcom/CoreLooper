#include "main.h"

MainLooper::MainLooper()
{
	SetMainLooper(this);
}

void MainLooper::OnCreate()
{
	__super::OnCreate();

	//PostQuitMessage();
	DV("SetTimer");
	SetTimer(mTimerDelayExit, 1 * 1000);
	DV("mTimerDelayExit=%d", mTimerDelayExit);
	//SetTimer(mTimerTest, 1000);
}

void MainLooper::OnTimer(long id)
{
	DV("%s,id=%d", __func__,id);

	if(id == mTimerTest)
	{
		static int idx = -1;
		++idx;
		DV("%s,idx=%04d", __func__, idx);
		return;
	}
	else if(id==mTimerDelayExit)
	{
		DV("mTimerDelayExit");
		PostQuitMessage(0);
		return;
	}

	__super::OnTimer(id);
}

int main()
{
	auto looper = make_shared<MainLooper>();
	looper->StartRun();
	return looper->GetQuitCode();
}
