#include "../core/base/base.inl"
using namespace Core;
class MainLooper :public Looper
{
	SUPER(Looper)
public:
	MainLooper()
	{
		SetMainLooper(this);
	}
protected:
	enum
	{
		eTimerDelayExit,
		eTimerTest,
		eTimerTestBase,
	};

	void OnCreate()
	{
		__super::OnCreate();

		SetTimer(eTimerDelayExit, 10 * 1000);
		SetTimer(eTimerTest, 1000);

		if (0)
		{
			int start = eTimerTestBase;
			int count = 1000 * 1000;
			DV("begin create timer,count=%d", count);
			for (int i = start; i < start + count; i++)
			{
				SetTimer(i, 1 + (i % 60000));
			}
			DV("end   create timer,count=%d", count);
		}
	}

	void OnTimer(long id)
	{
		switch (id)
		{
		case eTimerTest:
		{
			static int idx = -1;
			++idx;
			DV("%s,idx=%04d", __func__, idx);
			return;
		}
		case eTimerDelayExit:
		{
			PostQuitMessage(0);
			return;
		}
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
