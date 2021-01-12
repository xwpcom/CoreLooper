#pragma once
#include "core/looper/loop.h"
#include "core/looper/looperimpl.inl"

#ifdef _DEBUG
//#define _CONFIG_CALC_EVER_MAX_SIZE
#endif

namespace Bear {
namespace Core
{
class Looper;
struct tagLooperInternalData;
//XiongWanPing 2016.03.27
//各平台的Looper_XXX基类
class CORE_EXPORT LooperImpl :public Loop
{
	friend class Looper_Windows;
	friend class Looper_Linux;//for pclinux,android and ios
	friend class Looper;
	friend class TimerManager;
	friend struct tagHandlerInternalData;
	friend struct tagLooperInternalData;

	SUPER(Loop)
public:
	LooperImpl();
	virtual ~LooperImpl();

	static LooperImpl *CurrentLooper();
	static void SetCurrentLooper(LooperImpl *);
	static shared_ptr<Handler> Object(const string& url);

	virtual int Start();//在新创建的线程中运行
	virtual int StartRun();//在当前OS原生线程中直接运行(不能在looper环境中)
	int SetOwnerLooper(std::weak_ptr<Looper> owner);
	std::shared_ptr<Looper> GetOwnerLooper()const;
	virtual std::shared_ptr<Event> CreateExitEvent();

	int SetExitEvent(std::shared_ptr<Event> exitEvent);
	std::shared_ptr<Event> GetExitEvent()const;
	LPVOID GetLooperHandle()const
	{
		auto value = (LPVOID)(LONGLONG)mLooperHandle;
		return (LPVOID)value;
	}
	virtual bool IsQuiting()const;
	bool IsRunning()const;

	bool IsCurrentThread()const
	{
		return mThreadId == ShellTool::GetCurrentThreadId();
	}

	void LOOPER_SAFE Create(std::shared_ptr<Handler> parent=nullptr);
	virtual void LOOPER_SAFE Destroy();
	virtual int  LOOPER_SAFE PostQuitMessage(long exitCode = 0);
	virtual int GetQuitCode()const;
	virtual ULONGLONG tick()const
	{
		return mLooperTick;
	}
	virtual LRESULT sendMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
	virtual LRESULT postMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
	virtual LRESULT sendMessage(std::shared_ptr<Handler>handler, UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
	virtual LRESULT postMessage(std::shared_ptr<Handler>handler, UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);

	virtual void CancelRunnableInQueue(shared_ptr<Handler>& handler,shared_ptr<Runnable>& runnable);

#ifdef _DEBUG
	static int mTestState;
	static void SetTestState(int state);
	static int GetTestState();
#endif

protected:
	virtual long SetTimer(long& timerId, UINT interval);
	virtual void KillTimer(long& timerId);
	virtual long SetTimer(std::shared_ptr<Handler>handler, UINT interval);
	virtual void KillTimer(std::shared_ptr<Handler>handler, long& timerId);

	virtual long SetTimer(Handler *handler, UINT interval);//为了支持在Handler子类的构造函数中能调用SetTimer,构造函数中不能使用shared_from_this()
	virtual void KillTimer(Handler * handler, long& timerId);//为了支持在Handler子类的构造函数中能调用KillTimer

	void sendMessageHelper(tagLoopMessageInternal& msg, LooperImpl& looper);
	virtual bool PostQueuedCompletionStatus(HANDLE handle, DWORD bytes = 0, ULONG_PTR key = 0, LPOVERLAPPED lpOverlapped = 0) = 0;
	virtual void _StackLooperSendMessage(tagLoopMessageInternal& loopMsg) = 0;
	virtual int ProcessTimer(DWORD& cmsDelayNext,ULONGLONG ioIdleTick);

	virtual long SetTimerEx(UINT interval, std::shared_ptr<tagTimerExtraInfo> info = nullptr);
	virtual long SetTimerEx(std::shared_ptr<Handler>handler, UINT interval, std::shared_ptr<tagTimerExtraInfo> info = nullptr);
	virtual long SetTimerEx(Handler *handler, UINT interval, std::shared_ptr<tagTimerExtraInfo> info = nullptr);

	static LPVOID WINAPI _WorkThreadCB(LPVOID p);
	virtual long _WorkThread();

	virtual int StartHelper(bool newThread) = 0;

	virtual int Run();
	virtual void SingleStep();
	virtual int getMessage(tagLoopMessageInternal& msg) = 0;
	virtual LRESULT OnThreadMessage(tagLoopMessageInternal& msg);
	virtual LRESULT dispatchMessage(tagLoopMessageInternal& msg);

	virtual bool CanQuitLooperNow();
	virtual LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void OnTimer(long timerId);
	virtual void OnBMQuit();
	virtual int Wakeup();

	virtual void OnDestroy();

	virtual std::shared_ptr<Event> CreateExitEvent_Impl();

protected:
	HANDLE	mLooperHandle;
	ULONGLONG mLooperTick=0;
	ULONGLONG mLastIoTick = 0;/* 上次io发生的时间 */
	std::string	mThreadName;

	std::shared_ptr<TimerManager> GetTimerManager();
	std::shared_ptr<TimerManager> mTimerManager;

	std::shared_ptr<tagLooperInternalData> mLooperInternalData;
};

//FindObject in current looper
#define _Object(className,url) dynamic_pointer_cast<className>(Looper::Object(url))

//FindObject in main looper
#define _MObject(className,url)  dynamic_pointer_cast<className>(Looper::GetMainLooper()?Looper::GetMainLooper()->FindObject(url):nullptr)


}
}