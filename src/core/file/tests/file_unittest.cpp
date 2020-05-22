#include "../core/base/base.inl"
using namespace Bear::Core;
class MainLooper :public MainLooper_
{
	SUPER(MainLooper_)
protected:
	long  mTimerDelayExit = 0;
	long  mTimerTest = 0;
	long  mTimerTestBase = 0;

	void OnCreate()
	{
		__super::OnCreate();

		SetTimer(mTimerDelayExit, 10 * 1000);
		SetTimer(mTimerTest, 1000);
	}

	void OnTimer(long id)
	{
		if(id == mTimerTest)
		{
			static int idx = -1;
			++idx;
			DV("%s,idx=%04d", __func__, idx);
			return;
		}
		else if(id==mTimerDelayExit)
		{
			PostQuitMessage(0);
			return;
		}

		__super::OnTimer(id);
	}
};

int main()
{
	auto looper = make_shared<MainLooper>();
	looper->StartRun();

	return 0;
}
