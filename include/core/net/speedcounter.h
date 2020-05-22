#pragma once
namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2016.07.18
//计速器，可用来做网络流量监测
class CORE_EXPORT SpeedCounter
{
public:
	SpeedCounter()
	{
		mCount = 0;
		mTick = ShellTool::GetTickCount64();
	}

	virtual ~SpeedCounter() {}

	void Add(DWORD count)
	{
		mCount += count;
	}

	void Reset(DWORD &count, DWORD& interval)
	{
		ULONGLONG tickNow = ShellTool::GetTickCount64();
		count = mCount;
		interval = (DWORD)(tickNow - mTick);

		mCount = 0;
		mTick = tickNow;
	}

protected:
	DWORD		mCount;
	ULONGLONG 	mTick;
};

}
}
}
