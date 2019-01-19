#pragma once
#include <map>
#include <unordered_map>
#include "base/sigslot.h"
#include "base/object.h"
namespace Bear {
namespace Core
{

using std::list;
using std::string;
using std::map;
using std::unordered_map;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::dynamic_pointer_cast;

struct tagHandlerInternalData;
#define CHILD_NODE_NAME "Child"

enum eCoreMessage
{
	//app自定义的message建议从0开始,必须<BM_ALLOC_MESSAGE_ID_BASE

	BM_ALLOC_MESSAGE_ID_BASE = 10000,//由Handler::AllocMessageId分配的id起点
	
	//corelooper框架内部使用或保留>=BM_NULL的message
	BM_NULL = 1000000,
	BM_CREATE,			//主动调用Create()或AddChild时触发
	BM_DESTROY,			//调用Destroy()时触发

	//only for debug#begin
	BM_DUMP= BM_NULL+1000,
	BM_DUMP_ALL,
	BM_DUMP_PROC_DATA,
	//only for debug#end
};

//仿android Runnable命名
class Runnable :public enable_shared_from_this<Runnable>
{
public:
	virtual ~Runnable() {}

	virtual void Run() = 0;
};

//仿android message
class Message:public Object
{
public:
	LONGLONG	what = 0;
	LONG_PTR	arg1 = 0;
	LONG_PTR	arg2 = 0;
	shared_ptr<Bundle> mBundle;
	shared_ptr<Object> mObject;
};

struct tagDelayedRunnable;
struct tagTimerExtraInfo;
struct tagTimerNode;
class Handler;
class Loop;
class TimerManager;

struct tagAutoObject
{
public:
	virtual ~tagAutoObject() {}
	virtual void clear() {}
};

template<class T>
struct tagPostDisposeInfo :public tagAutoObject
{
	virtual ~tagPostDisposeInfo()
	{
		//DV("%s,this=%p",__func__,this);
	}

	void clear()
	{
		mSelfRef = nullptr;
	}

	shared_ptr<tagPostDisposeInfo> mSelfRef;
	shared_ptr<T> mSmartPtr;

};

//约定:
//采用LOOPER_SAFE修饰的接口可以安全的跨looper调用
//没有采用LOOPER_SAFE修饰的接口，不保证跨looper安全调用，应该只在handler所在looper里调用
#define LOOPER_SAFE	//表示接口是跨looper安全的

//XiongWanPing 2016.01.21
class CORE_EXPORT Handler :public Object
	, public enable_shared_from_this<Handler>
	, public sigslot::has_slots<>
{
	friend class Looper;
	friend class LooperImpl;
	friend class Looper_Windows;
	friend class Looper_Linux;
	friend class TimerManager;
	friend struct tagHandlerInternalData;
	friend struct tagLooperInternalData;
	friend class HandlerEx;
public:
	Handler();
	virtual ~Handler();

	virtual void LOOPER_SAFE Create(shared_ptr<Handler> parent);
	virtual void LOOPER_SAFE Destroy();

	//关于下面几个函数命名风格的说明
	//win32 api存在SendMessage,
	//在looper中向windows窗口调用SendMessage时可导致死锁(原因和解决方法详见CoreLooper用户手册)
	//为了便于在代码中发现错误调用windows SendMessage,采用sendMessage来区分,并且对SendMessage做了拦截处理
	//为了和sendMessage风格统一，联系比较紧密的几个api也采用此风格来命名
	//除此之外，都应该采用ThisStyle风格

#ifdef _MSC_VER
	/*
	Looper内部没有处理win32 message,和win32 thread互相send message时容易死锁,所以强烈建议使用PostMessage
	可能的死锁场景:
	win32Thread调用looper::sendMessage发消息给looper,此时win32Thread阻塞在looper::sendMessage()里面
	looper在处理消息时调用win32::SendMessage给win32Thread,但此时win32Thread所在的looper::sendMessage()没法再响应消息，造成死锁
	win32Thread和win32Thread之间互相调用SendMessage对发消息,looper和looper之间互相调用sendMessage对发消息，都不会造成死锁
	win32Thread和looper仅在同一消息处理中互相对发消息才会死锁，如果只有其中一方发消息，另一方可直接处理消息(即不需要对发消息)时，也不会死锁
	问题的根源在于:
	当looperThread调用win32::SendMessage时会完全阻塞等待win32线程响应消息，此时looperThread没法再响应新的looper消息
	如果恰在此时,win32Thread又调用looper::sendMessage,就会导致死锁
	可能的解决办法:待研究
	借助一个HelperLooper线程来调用win32::SendMessage,而win32决不会向此HelperLooper直接发送消息,此方法只能缓解一次对发消息，没法处理多次级联对发
	*/
	LRESULT SendMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		//本函数用来拦截误调用win32 SendMessage,但没法拦截::SendMessage
		DW("In order to avoid potential deadlock,please use PostMessage to win32 thread");
		ASSERT(FALSE);
		return -1;
	}
#endif

	//windows style
	virtual LRESULT LOOPER_SAFE sendMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
	virtual LRESULT LOOPER_SAFE postMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);

	//android style
	virtual LRESULT LOOPER_SAFE sendRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postDelayedRunnable(shared_ptr<Runnable> obj, UINT ms);
	virtual void    LOOPER_SAFE cancelRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE sendMessage(shared_ptr<Message> message);
	virtual LRESULT LOOPER_SAFE postMessage(shared_ptr<Message> message);
	//CoreLooper can implement all features of Android.Handler if necessary

	virtual long SetTimer(long& timerId, UINT interval);
	virtual void KillTimer(long& timerId);

	//延迟清除对象，可避免提前清除当前调用栈中还在引用的对象
	template<class T> void LOOPER_SAFE PostDispose(shared_ptr<T> & sp)
	{
		if (!IsCreated())
		{
			DW("fail %s", __func__);
			return;
		}
		if (!sp)
		{
			return;
		}

		auto obj = dynamic_pointer_cast<Handler>(sp);
		if (obj)
		{
			if (obj->IsDestroyed())
			{
				//do nothing
			}
			else
			{
				obj->MarkDestroyed();
				obj->sendMessage(BM_DESTROY);
			}
		}

		auto info = make_shared<tagPostDisposeInfo<T>>();
		info->mSelfRef = info;
		info->mSmartPtr.swap(sp);
		auto ret=PostDisposeHelper((WPARAM)info.get());
		if (ret)
		{
			info->mSelfRef = nullptr;
		}
	}

	virtual int LOOPER_SAFE AddChild(weak_ptr<Handler> child, string name = "");
	shared_ptr<Handler> LOOPER_SAFE GetParent()const;
	string LOOPER_SAFE GetUrl()const;
	virtual LONG_PTR LOOPER_SAFE GetId()const;
	virtual void LOOPER_SAFE SetId(LONG_PTR id);

	static Loop *GetCurrentLooper();
	string LOOPER_SAFE GetObjectName()const;
	void LOOPER_SAFE SetObjectName(const string& name);

	bool LOOPER_SAFE IsLooper()const;
	virtual shared_ptr<Handler> LOOPER_SAFE FindObject(const string& url);
	virtual shared_ptr<Handler> LOOPER_SAFE GetChild(LONG_PTR id);
	
	int LOOPER_SAFE RegisterShortcut(const string& name, weak_ptr<Handler> obj);
	shared_ptr<Handler> LOOPER_SAFE Shortcut(const string& name);

#define _Shortcut(className,name) dynamic_pointer_cast<className>(Shortcut(name))

	//可能阻塞很长时间,比如在looper中调用p2p api
	virtual bool LOOPER_SAFE MaybeLongBlock()const;
protected:
	void *operator new(size_t) = delete; //disable new,please use make_shared
	typedef LRESULT(Handler::*PFN_OnMessage)(WPARAM wp, LPARAM lp);


	UINT AllocMessageId();
	int BindMessageEx_(UINT msg, PFN_OnMessage entry);
	UINT BindMessage_(PFN_OnMessage entry);
#define BindMessage(entry) BindMessage_((PFN_OnMessage)entry)
#define BindMessageEx(msg,entry) BindMessageEx_(msg, (PFN_OnMessage)entry)

	virtual bool IsCreated()const;
	virtual bool IsDestroyed()const;
	void MarkDestroyed();
	void GetChildren(unordered_map<long*, weak_ptr<Handler>>& items);
	LPVOID GetLooperHandle();

	virtual shared_ptr<Handler> GetChild(string name, Handler *afterHandler = nullptr);
	DWORD GetThreadId()const
	{
		return mThreadId;
	}

	bool LOOPER_SAFE IsMyselfThread()const
	{
		DWORD tid = ShellTool::GetCurrentThreadId();
		return mThreadId == tid;
	}

	virtual LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void OnCreate();
	virtual void OnDestroy();
	virtual void OnTimer(long /*timerId*/) {}

	//主动直接调用OnTimer,并且不影响框架对OnTimer的调度
	virtual void PerformTimer(long timerId) { OnTimer(timerId); }

	virtual void HandleMessage(shared_ptr<Message> obj);

	virtual void OnPrepareDestructor();

	DWORD mThreadId = 0;
	shared_ptr<tagHandlerInternalData> mInternalData;
private:
	Handler & operator=(Handler& src);
	Handler(const Handler&);

	int PostDisposeHelper(WPARAM wp);
};

#ifdef _DEBUG
//only for debug,maybe not thread safe,so DO NOT use it in release version
class CORE_EXPORT HandlerDebug
{
public:
	static void DumpAll();
};
#endif

}
}