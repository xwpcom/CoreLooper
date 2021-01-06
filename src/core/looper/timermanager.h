#pragma once
#include <stdint.h>
#include <list>
#include "handler.h"
#include "timernode.h"

namespace Bear {
namespace Core
{

struct tagWheel;

//XiongWanPing 2016.07.04
class TimerManager
{
public:
	TimerManager();
	virtual ~TimerManager();
	int ProcessTimer(DWORD& cmsDelayNext,ULONGLONG tick);
	void clearCacheTick();

	int SetTimer(std::shared_ptr<Handler>handler, long timerId, UINT interval, std::shared_ptr<tagTimerExtraInfo> extraInfo = nullptr);
	void KillTimer(std::shared_ptr<Handler>handler, long& timerId);

	int SetTimer(Handler *handler, long timerId, UINT interval, std::shared_ptr<tagTimerExtraInfo> extraInfo = nullptr);
	void KillTimer(Handler *handler, long& timerId);

	void RemoveTimer(tagTimerNode* node);
	void EnableDebugInfo(bool enable)
	{
		mEnableDebugInfo = enable;
	}
private:
	long GetMinIdleTime()const;
	void DetectList();

	uint32_t Cascade(uint32_t wheelIndex);
	void AddTimer(uint32_t milseconds, tagTimerNode *node);
	void AddToReady(tagTimerNode *node);
	void ProcessTimeOut();

	static uint64_t GetCurrentMillisec();

private:
	tagWheel *  mWheels[5];
	uint64_t	mStartTick;
	tagNodeLink	mReadyNodes;
	bool		mBusying;
	bool		mEnableDebugInfo = false;
	ULONGLONG   mCacheTick=0;
};
}
}