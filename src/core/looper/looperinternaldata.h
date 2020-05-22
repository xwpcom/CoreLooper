#pragma once
namespace Bear {
namespace Core
{
class LooperImpl;
struct tagLooperInternalData
{
	tagLooperInternalData(LooperImpl*);
	~tagLooperInternalData();
	void gc();

	LooperImpl			*mLooperImpl=nullptr;
	bool				mLooperRunning = false;//looper数量不会很多，所以没有必要用bit field
	bool				mAttachThread = false;//不创建_WorkThreadCB线程，而是attach到其他线程中
	bool				mBMQuit = false;//是否已收到BM_QUIT消息
	std::shared_ptr<Event>	mExitEvent;//退出线程时可触发此事件

	CriticalSection					mMessageLock;
	std::list<tagLoopMessageInternal> 	mMessageList;

#ifdef _CONFIG_CALC_EVER_MAX_SIZE
	//统计曾经出现过的最大消息数，可用来评估系统性能
	long mCurrentMessages;
	long mEverMaxMessages;
#endif

	std::vector<std::shared_ptr<Event>>	mExitEvents;
	std::weak_ptr<Looper>		mOwnerLooper;
	long						mExitCode = 0;
	long						mTimerCheckQuitLooper = 0;
	LONGLONG					mTickStartQuit = 0;//用来计算此looper花多久才真正退出

	//private:
	std::map<void*, std::shared_ptr<Handler>> mDestroyedHandlers;
	long	mTimerGC = 0;
};

}
}
