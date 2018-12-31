#pragma once
namespace Bear {
namespace Core
{

class Runnable;
struct tagTimerExtraInfo
{
	tagTimerExtraInfo() {}
	virtual ~tagTimerExtraInfo() {}

	std::shared_ptr<Runnable> mRunnable;
};

struct tagDelayedRunnable
{
	virtual ~tagDelayedRunnable() {}

	std::shared_ptr<tagDelayedRunnable> mSelfRef;
	std::shared_ptr<Handler>  mHandler;
	std::shared_ptr<Runnable> mRunnable;
	UINT mDelayedMS = 0;//为0表示立即执行
};

struct tagSelfRef
{
	virtual ~tagSelfRef() {}

	std::shared_ptr<tagSelfRef> mSelfRef;
	std::shared_ptr<Object>		mObject;
};


}
}
