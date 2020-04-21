#include "stdafx.h"
#include "looper/looperimpl.h"
#include "timermanager.h"
#include "timerextrainfo.h"
#include "looper.h"
#include "base/stringtool.h"
#include "base/object.h"
#include "message.inl"
#include "handlerinternaldata.h"
#include "looperinternaldata.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

namespace Bear {
namespace Core
{
#define TAG "LooperImpl"
#ifdef _MSC_VER
static __declspec(thread) LooperImpl *gBaseLooper = NULL;
#else
#define CloseHandle close
static __thread LooperImpl *gBaseLooper = NULL;
#endif

struct tagCreateExitEventInfo
{
	shared_ptr<Event> mEvent;
};

LooperImpl *LooperImpl::CurrentLooper()
{
	if (!gBaseLooper)
	{
		//DV("gBaseLooper is null");
	}

	return gBaseLooper;
}

bool LooperImpl::IsQuiting()const
{
	return mLooperInternalData->mBMQuit;
}

bool LooperImpl::IsRunning()const
{
	return mLooperInternalData->mLooperRunning;
}

void LooperImpl::SetCurrentLooper(LooperImpl *looper)
{
	if (looper)
	{
		ASSERT(!gBaseLooper);
	}

	gBaseLooper = looper;
}

shared_ptr<Handler> LooperImpl::Object(const string& url)
{
	if (Looper::CurrentLooper())
	{
		return Looper::CurrentLooper()->FindObject(url);
	}

	ASSERT(FALSE);
	return nullptr;
}

LooperImpl::LooperImpl()
{
	mLooperInternalData = make_shared<tagLooperInternalData>(this);

#ifdef _DEBUG
	DV("%s(%s),this=%p", __func__,mThreadName.c_str(),this);
#endif
	mInternalData->mIsLooper = true;
	mThreadId = 0;
	mLooperHandle = INVALID_HANDLE_VALUE;

#ifdef _CONFIG_CALC_EVER_MAX_SIZE
	mCurrentMessages = 0;
	mEverMaxMessages = 0;
#endif

	mInternalData->SetActiveObject();
}

LooperImpl::~LooperImpl()
{
	//在非looper线程中发消息给looper时会创建临时looper
	//如果需要频繁的从非looper线程中发looper消息,可调用Looper::BindTLSLooper()来绑定
	//DV("%s,name=%s", __func__, mThreadName.c_str());//这里打印消息，可避免无意中创建太多临时looper,影响性能.
	LONGLONG tick = 0;
	if (mLooperInternalData->mTickStartQuit)
	{
		tick=ShellTool::GetTickCount64() - mLooperInternalData->mTickStartQuit;
	}

	DV("%s(%s),this=%p,quit tick=%lld", __func__, mThreadName.c_str(), this,tick);

	ASSERT(!mLooperInternalData->mLooperRunning);
	ASSERT(mLooperInternalData->mExitEvents.size() == 0);

	if (mLooperHandle != (HANDLE)INVALID_HANDLE_VALUE)
	{
		CloseHandle(mLooperHandle);
		mLooperHandle = (HANDLE)INVALID_HANDLE_VALUE;
	}

#ifdef _CONFIG_CALC_EVER_MAX_SIZE
	DV("mEverMaxMessages = %d", mEverMaxMessages);
#endif

	auto& msgList = mLooperInternalData->mMessageList;
	bool messageEmpty = msgList.empty();

	if (!messageEmpty)
	{
		bool fatalError = true;
		auto nc = msgList.size();
		tagLoopMessageInternal msg;
		if (nc == 1)
		{
			msg = msgList.front();
			if (msg.mMsg == BM_NULL)
			{
				fatalError = false;
			}
		}

		if (fatalError)
		{
			DW("fatal error,message pending in queue,please check app logic,nc=%d", nc);
			if (!mLooperInternalData->mAttachThread)
			{
				ASSERT(FALSE);
			}
		}
	}

	//ASSERT(messageEmpty);
}

LRESULT LooperImpl::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_HANDLER_DESTROY:
	{
		auto handler = (Handler*)wp;
		auto obj = handler->shared_from_this();
		mLooperInternalData->mDestroyedHandlers[handler] = obj;

		if (!mLooperInternalData->mTimerGC)
		{
			SetTimer(mLooperInternalData->mTimerGC,1000);
		}

		auto count = mLooperInternalData->mDestroyedHandlers.size();
		if (count > 10)
		{
			LogV(TAG,"this=%p,gc size=%d", this, count);
		}

		return 0;
	}
	case BM_POST_DISPOSE:
	{
		auto info = (Core::tagAutoObject*)wp;
		info->clear();
		mLooperInternalData->gc();
		return 0;
	}

	case BM_QUIT:
	{
		if (!mLooperInternalData->mBMQuit)
		{
			mLooperInternalData->mExitCode = (long)wp;
			mLooperInternalData->mTickStartQuit = ShellTool::GetTickCount64();
			postMessage(BM_DESTROY);
			OnBMQuit();
		}
		break;
	}
	case BM_CREATE_EXIT_EVENT:
	{
		tagCreateExitEventInfo *info = (tagCreateExitEventInfo *)wp;
		info->mEvent = CreateExitEvent_Impl();
		return 0;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

void LooperImpl::OnTimer(long id)
{
	if (id == mLooperInternalData->mTimerGC)
	{
		mLooperInternalData->gc();
		return;
	}

	__super::OnTimer(id);
}

void LooperImpl::OnBMQuit()
{
	ASSERT(!mLooperInternalData->mBMQuit);
	mLooperInternalData->mBMQuit = true;
	//DV("%s,mBMQuit=%d",mThreadName.c_str(),mBMQuit);

	ASSERT(mLooperInternalData->mTimerCheckQuitLooper == 0);
	if(mLooperInternalData->mTimerCheckQuitLooper == 0)
	{
		//注意:如果CanQuitLooperNow()返回false时,并且后续动作不能主动触发looper,
		//则需要主动定时触发CanQuitLooperNow()，否则looper有可能死等待
		mLooperInternalData->mTimerCheckQuitLooper = SetTimerEx(10);//定时检查CanQuitLooperNow()
	}
}

//looper析构函数执行比较慢时,app退出和looper析构可能竞争,即使采用shared_ptr也不能解决
//所以通过事件来解决，在app退出之前等待事件触发
int LooperImpl::SetExitEvent(shared_ptr<Event> exitEvent)
{
	if (IsRunning())
	{
		ASSERT(FALSE);
		return -1;
	}

	mLooperInternalData->mExitEvent = exitEvent;
	return 0;
}

void LooperImpl::Create(shared_ptr<Handler> parent)
{
	auto ret=Start();
	if (ret == 0)
	{
		if (parent)
		{
			parent->AddChild(shared_from_this());
		}
	}
}

void LooperImpl::Destroy()
{
	PostQuitMessage(0);
}

int LooperImpl::PostQuitMessage(long exitCode)
{
	if (!IsRunning())
	{
		return 0;
	}

	postMessage(BM_QUIT, (WPARAM)exitCode);
	return 0;
}

void LooperImpl::KillTimer(long& timerId)
{
	KillTimer(this, timerId);
}

long LooperImpl::SetTimer(long& timerId, UINT interval)
{
	if (timerId)
	{
		KillTimer(timerId);
	}

	return timerId = SetTimer(this,interval);
}

long LooperImpl::SetTimerEx(UINT interval, shared_ptr<tagTimerExtraInfo> info)
{
	return SetTimerEx(this, interval, info);
}

void LooperImpl::OnDestroy()
{
	__super::OnDestroy();
	//DV("%s(%s)", __func__, mThreadName.c_str());
}

int LooperImpl::Wakeup()
{
	postMessage(BM_NULL);
	return 0;
}

long LooperImpl::SetTimer(Handler *handler, UINT interval)
{
	return SetTimerEx(handler, interval);
}

void LooperImpl::KillTimer(Handler *handler, long& timerId)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return ;
	}
	//long ret = -1;

	if (!IsCurrentThread())
	{
		ASSERT(FALSE);
		return ;
	}

	auto timerMan = LooperImpl::GetTimerManager();
	timerMan->KillTimer(handler, timerId);
	//return 0;
}

long LooperImpl::SetTimerEx(Handler *handler, UINT interval, shared_ptr<tagTimerExtraInfo> info)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (!IsCurrentThread())
	{
		ASSERT(FALSE);
		return 0;
	}

	if (!handler->IsCreated())
	{
		DW("fail set timer,handler is NOT created");
		return 0;
	}

	if (!IsLooper())
	{
		if (handler->IsDestroyed())
		{
			if (mLooperInternalData->mDestroyedHandlers.find(handler) == mLooperInternalData->mDestroyedHandlers.end())
			{
				DW("fail set timer,handler is destroyed");
				return 0;
			}
		}
	}

	auto timerMan = GetTimerManager();
	auto timerId = handler->mInternalData->NextTimerId();
	timerMan->SetTimer(handler, timerId, interval, info);
	postMessage(BM_NULL);//投递消息保证重新计算等待时间
	return timerId;
}

long LooperImpl::SetTimer(shared_ptr<Handler>handler, UINT interval)
{
	return SetTimerEx(handler, interval);
}

void LooperImpl::KillTimer(shared_ptr<Handler>handler, long& timerId)
{
	if (!handler)
	{
		return ;
	}

	KillTimer(handler.get(), timerId);
}

long LooperImpl::SetTimerEx(shared_ptr<Handler>handler, UINT interval, shared_ptr<tagTimerExtraInfo> info)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return 0;
	}

	return SetTimerEx(handler.get(), interval, info);
}

int LooperImpl::SingleStep()
{
	tagLoopMessageInternal msg;
	int ret = getMessage(msg);
	if (ret == 0)
	{
#ifdef _CONFIG_DEBUG_LOOPER
		auto tick = ShellTool::GetTickCount64();
		string name;
		UINT msgId = 0;
		const bool isMainLooper = Looper::IsMainLooper(this);
		if (isMainLooper)
		{
			if (msg.mHandler)
			{
				name = msg.mHandler->GetName();
				msgId = msg.mMsg;
			}
		}
#endif

		// 注意:这里不能用ret=dispatchMessage(msg)
		// message处理结果已经通过其他方式返回给message发送方
		dispatchMessage(msg);

#ifdef _CONFIG_DEBUG_LOOPER
#error _CONFIG_DEBUG_LOOPER
		if (isMainLooper)
		{
			tick = ShellTool::GetTickCount64() - tick;
			if (tick > 200)
			{
				DV("main looper too slow? %s.msg=%d,tick=" FMT_LONGLONG, name.c_str(), msgId, tick);
			}
		}
#endif
	}

	return ret;
}

LRESULT LooperImpl::dispatchMessage(tagLoopMessageInternal& msg)
{
	LRESULT ret = -1;
	if (msg.mHandler)
	{
		ret = msg.mHandler->OnMessage(msg.mMsg, msg.mWP, msg.mLP);
	}
	else
	{
		ret = OnThreadMessage(msg);
	}

	if (msg.mDone)
	{
		if (msg.mAck)
		{
			*msg.mAck = ret;
		}

		*msg.mDone = true;

		if (msg.mWaitAck && msg.mSourceBaseLoop)
		{
			ASSERT(msg.mSourceBaseLoop != shared_from_this());
			msg.mSourceBaseLoop->Wakeup();
		}
	}

	return ret;
}

LRESULT LooperImpl::OnThreadMessage(tagLoopMessageInternal& msg)
{
	ASSERT(!msg.mHandler);

	return OnMessage(msg.mMsg, msg.mWP, msg.mLP);
}

int LooperImpl::Run()
{
	while (1)
	{
		int ret = SingleStep();
		if (ret && mLooperInternalData->mBMQuit && CanQuitLooperNow())
		{
			AutoLock lock(&mLooperInternalData->mMessageLock);
			if (mLooperInternalData->mMessageList.size() == 0)
			{
				mLooperInternalData->mLooperRunning = false;
				break;
			}
		}
	}

	return 0;
}

int LooperImpl::Start()
{
	bool newThread = true;
	return StartHelper(newThread);
}

int LooperImpl::StartRun()
{
	if (CurrentLooper())
	{
		DW("");
		ASSERT(FALSE);
		return -1;
	}

	bool newThread = false;
	StartHelper(newThread);
	return GetQuitCode();
}

LRESULT LooperImpl::postMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	return postMessage(shared_from_this(), msg, wp, lp);
}

LRESULT LooperImpl::sendMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	auto ret = sendMessage(shared_from_this(), msg, wp, lp);
	return ret;
}

long LooperImpl::_WorkThread()
{
	if (!mThreadName.empty())
	{
		ShellTool::SetThreadName(mThreadName.c_str());
	}

	mThreadId = ShellTool::GetCurrentThreadId();
	//DV("%s::%s,mThreadId=%d", mThreadName.c_str(), __func__, mThreadId);

	//说明:在创建thread之前就要置mCreated = true;
	//如果到这里才置true,由于线程调度影响，可能导致looper->Start()之后mCreate短暂为false而导致竞争关系
	//影响是send/post会失败!
	//if (!IsCreated())
	{
		//mInternalData->mCreated = true;
		OnCreate();
	}

	Run();

	auto ret = mLooperInternalData->mExitCode;
	ASSERT(!mLooperInternalData->mLooperRunning);
	//ExitInstance();
	ASSERT(mInternalData->mSelfRef);
	mInternalData->mSelfRef = nullptr;//这里会自动调用delete this

	ShellTool::SetThreadName("UserWorkItem");

	return ret;
}

LPVOID LooperImpl::_WorkThreadCB(LPVOID p)
{
	LooperImpl *pThis = (LooperImpl*)p;
	gBaseLooper = pThis;

#ifdef _DEBUG
	//string name = pThis->GetObjectName();
#endif

	auto exitEvent = pThis->mLooperInternalData->mExitEvent;
	auto ret = pThis->_WorkThread();
	if (exitEvent)
	{
#ifdef _DEBUG

		//DV("SetEvent(%s)", name.c_str());
		//ShellTool::Sleep(3000);
#endif
		exitEvent->Set();//上层可等待此事件，确认looper完全退出
	}

	LPVOID res = (LPVOID)(LONGLONG)ret;
	return res;
}

LRESULT LooperImpl::postMessage(shared_ptr<Handler>handler, UINT msg, WPARAM wp, LPARAM lp)
{
	if (!mLooperInternalData->mLooperRunning && !mLooperInternalData->mAttachThread)
	{
		if(handler)
		{
			DW("skip postMessage(handler=%s,msg=%d)",handler->GetObjectName().c_str(),msg);
		}
		
		//ASSERT(FALSE);
		return 0;
	}

	tagLoopMessageInternal loopMsg;
	loopMsg.mHandler = handler;
	loopMsg.mMsg = msg;
	loopMsg.mWP = wp;
	loopMsg.mLP = lp;

	{
		//*
		if (mLooperInternalData->mAttachThread)
		{
			//stack looper没有消息循环,所以只触发，不加空消息
			if (msg == BM_NULL)
			{
				
			}
			else
			{
				ASSERT(FALSE);
			}
		}
		else
			//*/
		{
			AutoLock lock(&mLooperInternalData->mMessageLock);
			if (!mLooperInternalData->mLooperRunning)
			{
				return -1;
			}
			mLooperInternalData->mMessageList.push_back(loopMsg);
		}

		PostQueuedCompletionStatus(mLooperHandle);
	}

	return 0;
}


LRESULT LooperImpl::sendMessage(shared_ptr<Handler> handler, UINT msg, WPARAM wp, LPARAM lp)
{
	if (!mLooperInternalData->mLooperRunning)
	{
		//ASSERT(FALSE);
		DW("looper is NOT running,skip %s(msg=%d)", __func__, msg);
		return 0;
	}

	if (mLooperInternalData->mAttachThread && IsMyselfThread())
	{
		//清理stack looper收到的所有BM_NULL消息
		AutoLock lock(&mLooperInternalData->mMessageLock);
		auto& msgList = mLooperInternalData->mMessageList;
		while (!msgList.empty())
		{
			tagLoopMessageInternal& msg = msgList.front();
			msgList.pop_front();
			if (msg.mMsg == BM_NULL)
			{
				//int x = 0;
			}
			else
			{
				ASSERT(FALSE);
			}
		}
	}

	bool done = false;
	LRESULT ack = 0;//默认不能为-1,原因是app可能把返回值当指针
	tagLoopMessageInternal loopMsg;
	loopMsg.mHandler = handler;
	loopMsg.mMsg = msg;
	loopMsg.mWP = wp;
	loopMsg.mLP = lp;
	loopMsg.mDone = &done;
	loopMsg.mAck = &ack;

	//注意:在主线程调用looper Start()之后马上调用sendMessage时,有可能looper中的mThreadId在_WorkThread中还没初始化，但这不影响结果
	if (IsCurrentThread())
	{
		return dispatchMessage(loopMsg);
	}

	loopMsg.mWaitAck = true;

	{
		LooperImpl *looper = CurrentLooper();
		if (looper)
		{
			sendMessageHelper(loopMsg, *looper);
			return ack;
		}
	}

#if defined MSC_VER && !defined _DEBUG
	//Looper_Windows和win32 thread互相send message时容易死锁,所以要用postMessage
	ASSERT(FALSE);//please use postMessage
	return 0;
#endif

#ifdef _DEBUGx
	{
		//_StackLooperSendMessage比较低效，所以这里打印出一些信息,上层可据此优化，比如调用BindTlsLooper
		//已采用SmartTlsLooper优化
		string desc;
		if (handler)
		{
			desc += StringTool::Format("obj=%s", handler->GetObjectName().c_str());
		}

		desc += StringTool::Format(",msg=%u", msg);
		DV("_StackLooperSendMessage,desc=%s", desc.c_str());
	}
#endif

	_StackLooperSendMessage(loopMsg);
	return ack;
}

void LooperImpl::sendMessageHelper(tagLoopMessageInternal& msg, LooperImpl& looper)
{
	msg.mSourceBaseLoop = dynamic_pointer_cast<LooperImpl>(looper.shared_from_this());

	{
		AutoLock lock(&mLooperInternalData->mMessageLock);
		if (!mLooperInternalData->mLooperRunning)
		{
			return;
		}
		mLooperInternalData->mMessageList.push_back(msg);
	}

	PostQueuedCompletionStatus((HANDLE)mLooperHandle);
/*
#ifdef _DEBUG
	string info;
	info = StringTool::Format("%s,msg=%d", __func__, msg.mMsg);
	TickDumper check(info.c_str(), false, 100);
#else
	TickDumper check(__func__, false, 100);
#endif
*/
	while (!*msg.mDone)
	{
		looper.SingleStep();
	}
}

int LooperImpl::ProcessTimer(DWORD& cmsDelayNext)
{
	if (mTimerManager == nullptr)
	{
		return -1;
	}

	bool needWait = true;
	auto timerMan = LooperImpl::GetTimerManager();
	needWait = (timerMan->ProcessTimer(cmsDelayNext) == 0);

	if (needWait)
	{
		if (cmsDelayNext != INFINITE)
		{
			cmsDelayNext = MAX(1, cmsDelayNext);//发现timer不是很精确,不能恰好等那么长时间，也许可提高精度
		}

		ASSERT(cmsDelayNext);//如果为0，会形成busy loop,占用大量cpu
	}

	return needWait ? -1 : 0;
}

shared_ptr<TimerManager> LooperImpl::GetTimerManager()
{
	if (mTimerManager == nullptr)
	{
		mTimerManager = make_shared<TimerManager>();
	}

	return mTimerManager;
}

shared_ptr<Event> LooperImpl::CreateExitEvent_Impl()
{
	auto evt(make_shared<Event>(true, false));
	mLooperInternalData->mExitEvents.push_back(evt);
	return evt;
}

shared_ptr<Event> LooperImpl::CreateExitEvent()
{
	if (IsMyselfThread())
	{
		return CreateExitEvent_Impl();
	}
	else
	{
		tagCreateExitEventInfo info;
		sendMessage(BM_CREATE_EXIT_EVENT, (WPARAM)&info);
		return info.mEvent;
	}
}

bool LooperImpl::CanQuitLooperNow()
{
	ASSERT(mLooperInternalData->mBMQuit);

	auto& events = mLooperInternalData->mExitEvents;
	while (events.size())
	{
#ifdef _MSC_VER
		BOOL waitAll = TRUE;
		HANDLE arr[MAXIMUM_WAIT_OBJECTS];
		if (events.size() <= COUNT_OF(arr))
		{
			for (UINT i = 0; i < events.size(); i++)
			{
				arr[i] = events[i]->operator HANDLE();
			}

			int ret = WaitForMultipleObjects((DWORD)events.size(), arr, waitAll, 0);
			if (ret == WAIT_OBJECT_0)
			{
				events.clear();
				break;
			}
			else
			{
				return false;
			}
		}
		else
		{
			for (UINT i = 0; i < events.size(); i++)
			{
				bool signal = events[i]->Wait(0);
				//DV("events[%d],signal=%d",i,signal);
				if (signal)
				{
				}
				else
				{
					return false;
				}
			}

			events.clear();
		}
#else
		for (UINT i = 0; i < events.size(); i++)
		{
			bool signal = events[i]->Wait(0);
			//DV("events[%d],signal=%d",i,signal);
			if (signal)
			{
			}
			else
			{
				return false;
			}
		}

		events.clear();
#endif
	}

	{
		static int dumpTimes = 0;
		static ULONGLONG tick = ShellTool::GetTickCount64();
		ULONGLONG tickNow = ShellTool::GetTickCount64();
		if (tickNow >= tick + 2000)
		{
			++dumpTimes;
			if (dumpTimes == 1)
			{
				mInternalData->Dump(0);//dump which object are still live
			}
			tick = tickNow;
		}
	}

	//有未决事务时，应该等到所有事务完结后才能安全退出looper
	auto nc = mInternalData->GetLiveChildrenCount();
	return nc == 0;
}

int LooperImpl::SetOwnerLooper(weak_ptr<Looper> owner)
{
	if (mLooperInternalData->mOwnerLooper.lock())
	{
		ASSERT(FALSE);
		return -1;
	}

	mLooperInternalData->mOwnerLooper = owner;
	return 0;
}

void LooperImpl::CancelRunnableInQueue(shared_ptr<Handler> handler, shared_ptr<Runnable> runnable)
{
	if (!handler || !runnable)
	{
		return;
	}

	if (!IsMyselfThread())
	{
		ASSERT(FALSE);
		return;
	}

	{
		AutoLock lock(&mLooperInternalData->mMessageLock);

		auto& items = mLooperInternalData->mMessageList;
		for (auto iter2 = items.begin(); iter2 != items.end();)
		{
			auto iter = iter2++;

			if (iter->mMsg != BM_POST_RUNNABLE
				|| iter->mHandler != handler
				)
			{
				continue;
			}

			auto info = (tagDelayedRunnable*)iter->mWP;
			if (info->mRunnable == runnable)
			{
				items.erase(iter);
				info->mSelfRef = nullptr;

				//可能存在多个，所以要继续遍历
			}
		}
	}

}

shared_ptr<Looper> LooperImpl::GetOwnerLooper()const
{
	return mLooperInternalData->mOwnerLooper.lock();
}

shared_ptr<Event> LooperImpl::GetExitEvent()const
{
	return mLooperInternalData->mExitEvent;
}

int LooperImpl::GetQuitCode()const
{
	//compatible with int main(...)
	return (int)mLooperInternalData->mExitCode;
}

#ifdef _DEBUG
int LooperImpl::mTestState = -1;
void LooperImpl::SetTestState(int state)
{
	DV("%s,state=%d", __func__, state);
	mTestState = state;
}

int LooperImpl::GetTestState()
{
	return mTestState;
}
#endif

}
}
