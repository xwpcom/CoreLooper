#pragma once

namespace Bear {

namespace Core
{
class Handler;
class LooperImpl;
//tagLoopMessageInternal供内部实现时使用,后面再做成对外界不可见的
struct tagLoopMessageInternal :public tagLoopMessage
{
	friend class LooperImpl;
	friend class Looper_Windows;
	friend class Looper_Linux;

	tagLoopMessageInternal()
	{
	}

protected:
	bool	*mDone=nullptr;
	bool	mWaitAck=false;
	LRESULT *mAck=nullptr;
	std::shared_ptr<LooperImpl> mSourceBaseLoop=nullptr;//处理完消息后要通知mSourceBaseLoop
};

struct tagSetKillTimerInfo
{
	tagSetKillTimerInfo()
	{
		handler = NULL;
		timerId = 0;
		interval = 0;
	}

	std::shared_ptr<Handler>handler;
	UINT timerId;
	UINT interval;
	std::shared_ptr<tagTimerExtraInfo> extraInfo;
};

struct tagDelayedMessageInfo
{
	tagDelayedMessageInfo()
	{
		handler = nullptr;
		delayMS = 0;
		msg = 0;
		wp = 0;
		lp = 0;
	}

	std::shared_ptr<Handler>handler;
	UINT		delayMS;
	UINT		msg;
	WPARAM		wp;
	LPARAM		lp;
};
}
}