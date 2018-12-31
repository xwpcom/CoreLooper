#pragma once
namespace Bear {
namespace Core
{
class Handler;
class TimerManager;

typedef struct tagNodeLink
{
	tagNodeLink *prev;
	tagNodeLink *next;
	tagNodeLink() { prev = next = this; } //circle
}tagNodeLink;

typedef struct tagTimerNode
{
	tagNodeLink		link;
	TimerManager	*mTimerManager = nullptr;
	Handler*		mHandler = nullptr;
	uint64_t		mDeadTime = 0;
	long			mTimerId=0;
	uint32_t		mInterval = 0;

	std::shared_ptr<tagTimerExtraInfo> mExtraInfo;//用来实现延时消息

	tagTimerNode(TimerManager *tm, Handler *handler, uint64_t dt, uint32_t interval)
		: mTimerManager(tm)
		, mHandler(handler)
		, mDeadTime(dt), mInterval(interval)
	{}
}tagTimerNode;

}
}