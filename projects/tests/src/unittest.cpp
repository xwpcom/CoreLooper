#include "stdafx.h"
#include "CppUnitTest.h"
#include "derive.h"
#include "core/looper/teststate.h"
#include <atomic> 

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;


namespace TestBear
{
TEST_CLASS(UnitTest)
{
public:
	TEST_METHOD(TestStringTool)
	{
		auto obj = StringTool::Format("Hello,%d", 1);
		Assert::AreEqual(obj.c_str(), "Hello,1");
	}
};

static atomic_int gWorkLooperTimes = 0;

TEST_CLASS(Core)
{
public:
	TEST_METHOD(TestMessage)
	{
		//仿Android.os.Message
		class Message
		{
		public:
			Message()
			{

			}

			WPARAM arg1 = 0;
			LPARAM arg2 = 0;
			shared_ptr<Object> obj;
			LONG_PTR sendingUid = 0;
			LONG_PTR what = 0;
			shared_ptr<Bundle> bundle;

			static shared_ptr<Message> obtain()
			{
				return make_shared<Message>();
			}
		};

	}

	//TODO:有时vs test异常结束，原因待查
	TEST_METHOD(SendMessageSpeed)
	{
		static bool bindCPU = false;
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mObjectName = "MainLooper";
				mThreadName = mObjectName;
			}

		protected:
			int InitInstance()
			{
				if (bindCPU)
				{
					HANDLE hThread = OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, GetCurrentThreadId());
					DWORD_PTR value = SetThreadAffinityMask(hThread, 0x0001);
				}
				
				return __super::InitInstance();
			}
		};

		auto tlsLooper = Looper::BindTLSLooper();

		if (bindCPU)
		{
			HANDLE hThread = OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, GetCurrentThreadId());
			DWORD_PTR value = SetThreadAffinityMask(hThread, 0x0001);
		}

		auto event = make_shared<Event>();
		{
			auto obj = make_shared<MainLooper>();
			obj->SetExitEvent(event);
			auto ret = obj->Start();
			Assert::AreEqual(ret, 0);

			/*
			测试环境(公司台式机,i7 8核cpu)
			Release比Debug快很多
			绑定cpu比不绑定快很多

			Debug版
			Looper::sendMessage耗时17秒,每秒5.8万次

			Release版不绑定cpu,发送100万条消息
			Looper::sendMessage耗时4秒到8秒,每秒12.5到25万次

			Release版绑定cpu,发送100万条消息
			Looper::sendMessage耗时1.4秒左右,平均每秒能发送约71万次
			*/

			auto nc = 1000 * 1000;
			auto tick = ShellTool::GetTickCount64();
			for (int i = 0; i < nc; ++i)
			{
				obj->sendMessage(BM_NULL);
			}
			tick = ShellTool::GetTickCount64() - tick;
			DV("tick=%I64d", tick);
			obj->PostQuitMessage();
		}
		//event->Wait();
	}

	TEST_METHOD(TestLooper)
	{
		class MainLooper :public Looper
		{
		protected:
			int InitInstance()
			{
				PostQuitMessage(1);
				return __super::InitInstance();
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 0);
	}

	//确保能支持大量的Looper
	TEST_METHOD(TestManyLooper)
	{
		class WorkLooper :public Looper
		{
			int InitInstance()
			{
				++gWorkLooperTimes;
				return __super::InitInstance();

			}
		public:

		};

		int testLooperCount = 1000;

		class MainLooper :public Looper
		{
			SUPER(Looper)
		public:
			MainLooper()
			{
				SetMainLooper(this);

			}

			int mTestLooperCount = 0;
		protected:
			void OnCreate()
			{
				__super::OnCreate();


				int nc = mTestLooperCount;
				for (int i = 0; i < nc; i++)
				{
					auto obj = make_shared<WorkLooper>();
					AddChild(obj);
					obj->Start();
				}

				class Worker :public Runnable
				{
					void Run()
					{
						Looper::GetMainLooper()->PostQuitMessage();
					}
				};
				Looper::GetMainLooper()->PostQuitMessage();
				postDelayedRunnable(make_shared<Worker>(), 1);
			}
		};

		{
			auto obj = make_shared<MainLooper>();
			obj->mTestLooperCount = testLooperCount;
			obj->StartRun();
		}

		Assert::IsTrue(gWorkLooperTimes == testLooperCount);
	}

	TEST_METHOD(TestLooper_OnCreate)
	{
		class TestLooper :public Looper
		{
			SUPER(Looper)
		public:
			TestLooper()
			{
				mObjectName = "TestLooper";
				mThreadName = mObjectName;
			}
		protected:
			void OnCreate()
			{
				DV("%s", __func__);
				__super::OnCreate();

				auto obj = _MObject(TestLooper, "TestLooper");
				if (obj)
				{
					DW("obj is ok");
				}
				else
				{
					DW("obj is null");
				}
				int x = 0;
			}
			
			void OnDestroy()
			{
				DW("%s", __func__);
				__super::OnDestroy();
			}
			int InitInstance()
			{
				DV("%s", __func__);
				return __super::InitInstance();
			}
		};

		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mObjectName = "MainLooper";
				mThreadName = mObjectName;

				SetMainLooper(this);
			}
		protected:
			void OnCreate()
			{
				DV("%s", __func__);
				__super::OnCreate();

				{
					auto looper = make_shared<TestLooper>();
					AddChild(looper);
					looper->Start();
				}
			}
			void OnDestroy()
			{
				DW("%s", __func__);
				__super::OnDestroy();
			}
			int InitInstance()
			{
				DV("%s", __func__);
				PostQuitMessage(1);
				return __super::InitInstance();
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 0);
	}

	TEST_METHOD(SharedPtrPerformance)
	{
		class Test :public enable_shared_from_this<Test>
		{

		};

		//debug版的性能比release差几个数量级
		auto tick = ShellTool::GetTickCount64();
		auto nc = 1000 * 1000;
		int *p = nullptr;
		auto obj = make_shared<Test>();
		auto item = obj;
		//obj.count();
		for (auto i = 0; i < nc; i++)
		{
			//volatile int *p2 = p;
			item = obj;
		}

		tick = ShellTool::GetTickCount64() - tick;
		DV("tick=%I64d", tick);
	}

	//检查weak_ptr是否跨线程安全
	//测试办法:在object析构函数中发消息触发looper对此object weak_ptr进行lock,看是否成功
	//结果:经测试确认，一旦执行到object的析构，在其他looper中weak_ptr lock会返回nullptr
	TEST_METHOD(CheckWeakPtrLooperSafe)
	{
		enum
		{
			BM_LOCK,
		};

		class LockLooper :public Looper
		{
		public:
			weak_ptr<Derive> mHandler;
		protected:
			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				switch (msg)
				{
				case BM_LOCK:
				{
					ShellTool::Sleep(100);
					auto obj = mHandler.lock();
					if (obj)
					{
						DW("age=%d", obj->mAge);
					}
					else
					{
						DW("obj is null");
					}
					return 0;
				}
				}

				return __super::OnMessage(msg, wp, lp);
			}
		};

		{
			auto event = make_shared<Event>();
			{
				auto looper = make_shared<LockLooper>();
				looper->SetExitEvent(event);
				{
					{
						shared_ptr<Derive> obj = make_shared<Derive>();
						obj->mObject = obj;
						obj->mLockLooper = looper;

						looper->mHandler = obj;
						looper->Start();
					}
				}

				looper->PostQuitMessage();
			}
			event->Wait();
		}
	}

	//评估handler字节数,尽量优化
	TEST_METHOD(HandlerBytes)
	{
		class Handler2 :public Object
			, public enable_shared_from_this<Handler2> //16 bytes
			, public sigslot::has_slots<> ////+40 bytes
		{
		protected:
			//string sz;//+40bytes
		};

		DV("sizeof(string) = %d", sizeof(string));//40
		std::map<long*, weak_ptr<Handler>> mChildren;
		DV("sizeof(map) = %d", sizeof(mChildren));//24
		DV("sizeof(Handler) = %d", sizeof(Handler));
		DV("sizeof(Handler2) = %d", sizeof(Handler2));

		DV("sizeof(shared_ptr)=%d", sizeof(shared_ptr<std::map<UINT, shared_ptr<tagTimerNode>>>));
	}

	//测试定时器
	TEST_METHOD(Timer)
	{
		class MainLooper :public Looper
		{
			SUPER(Looper)
				//long mTimerInConstructor = 0;
		public:
			MainLooper()
			{
				SetMainLooper(this);
				//SetTimer(mTimerInConstructor,1000);//不能在构造函数中调用SetTimer
			}
		protected:

			void OnTimer(long id)
			{
				/*
				if (id == mTimerInConstructor)
				{
				DV("%s,mTimerInConstructor", __func__);
				return;
				}
				*/

				__super::OnTimer(id);
			}

			class WatchDog :public Handler
			{
			public:
				WatchDog() {}

			protected:
				void OnCreate()
				{
					__super::OnCreate();

					for (int i = 0; i < 5; i++)
					{
						DV("timerId#%d=%d", i + 1, SetTimer(2000));
					}
					//return;

					SetTimer(mTimerTest, 1000);
					SetTimer(mTimerFeed, 2000);
				}

				void OnTimer(long id)
				{
					//DV("%s,id=%d", __func__, id);

					if (id == mTimerTest)
					{
						static int idx = -1;
						++idx;
						DV("id=%d,test idx=%04d", id, idx);
						if (idx == 5)
						{
							KillTimer(mTimerTest);
							Looper::CurrentLooper()->PostQuitMessage(123);
						}
						return;
					}
					else if (id == mTimerFeed)
					{
						static int idx = -1;
						++idx;
						DV("id=%d,feedDog,idx=%04d", id, idx);
						return;
					}

					__super::OnTimer(id);
				}


				long mTimerTest = 0;
				long mTimerFeed = 0;
			};

			void OnCreate()
			{
				__super::OnCreate();

				AddChild(make_shared<WatchDog>());
			}
		};

		auto looper = make_shared<MainLooper>();
		auto ret = looper->StartRun();
		DV("exit code=%d", looper->GetQuitCode());
	}

	//XiongWanPing 2018.06.28
	//故意构造一个场景，让handler在OnTimer内部清除app对此handler的最后一个ref，测试是否会crash
	TEST_METHOD(DestroyHandlerInTimer)
	{
		class DelayLooper :public Looper
		{
		public:
			DelayLooper()
			{
				DV("%s", __func__);
			}
			~DelayLooper()
			{
				DV("%s", __func__);
			}

			class DelayHandler :public Handler
			{
			public:
				DelayHandler()
				{
					DV("%s", __func__);
				}
				~DelayHandler()
				{
					DV("%s", __func__);
				}
			protected:
				void OnDestroy()
				{
					DV("%s", __func__);
					__super::OnDestroy();

				}

				void OnCreate()
				{
					__super::OnCreate();

					mDelayRef = shared_from_this();
					mTimerDelay = SetTimer(3000);
				}

				void OnTimer(long id)
				{
					if (id == mTimerDelay)
					{
						DV("%s", __func__);
						mDelayRef = nullptr;
						KillTimer(mTimerDelay);
						return;
					}
					__super::OnTimer(id);
				}

				shared_ptr<Handler> mDelayRef;
				long mTimerDelay = 0;
			};

			void OnCreate()
			{
				__super::OnCreate();

				DV("%s", __func__);
				AddChild(make_shared<DelayHandler>());
				PostQuitMessage();
			}
		};

		auto looper = make_shared<DelayLooper>();
		auto ret = looper->StartRun();
	}

#ifdef _CONFIG_TEST_CROSS_LOOPER_WEAK_PTR_LOCK
	//XiongWanPing 2018.07.27
	TEST_METHOD(CrossLooperWeakPtrLock)
	{
		class WorkLooper :public Looper
		{
		public:
			enum
			{
				BM_CREATE_HANDLER,
			};
			struct tagCreateHandler
			{
				shared_ptr<Handler> mHandler;
			};

			class WorkHandler :public Handler
			{
			public:
				WorkHandler()
				{
					DW("%s", __func__);
				}
				~WorkHandler()
				{
					DW("%s", __func__);
				}
			protected:
				void OnCreate()
				{
					__super::OnCreate();
					
					SetTimer(mTimerTest, 1);
					SetTimer(mTimerDelayDestroy, 100);
				}

				void OnTimer(long id)
				{
					if (id == mTimerTest)
					{
						++mValue;//访问变量，确保handler是有效的
						//DV("this=%p,value=%d",this, mValue);
						return;
					}
					else if (id == mTimerDelayDestroy)
					{
						KillTimer(mTimerDelayDestroy);
						Destroy();
						return;
					}

					__super::OnTimer(id);
				}

				long mTimerTest = 0;
				long mTimerDelayDestroy = 0;
				int mValue=0;
			};

			void OnCreate()
			{
				__super::OnCreate();
				DV("%s", __func__);
				class DelayWorker :public Runnable
				{
				public:
					DelayWorker()
					{
						//DW("%s", __func__);
					}
					~DelayWorker()
					{
						//DW("%s", __func__);
					}
					void Run()
					{
						//DW("%s", __func__);
						Looper::GetMainLooper()->PostQuitMessage();
					}
				};

				postDelayedRunnable(make_shared<DelayWorker>(), 1000 * 10);
			}

			shared_ptr<WorkHandler> CreateWorkHandler()
			{
				tagCreateHandler info;
				sendMessage(BM_CREATE_HANDLER, (WPARAM)&info);
				return dynamic_pointer_cast<WorkHandler>(info.mHandler);
			}

			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				switch (msg)
				{
				case BM_CREATE_HANDLER:
				{
					auto info = (tagCreateHandler *)wp;
					info->mHandler = make_shared<WorkHandler>();
					AddChild(info->mHandler);
					return 0;
				}
				}

				return __super::OnMessage(msg, wp, lp);
			}
		};

		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				SetMainLooper(this);
			}
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				auto looper = make_shared<WorkLooper>();
				AddChild(looper);
				looper->Start();

				auto handler = looper->CreateWorkHandler();
				//DV("handler=%p", handler);
				mWeakHandler =handler;

				SetTimer(mTimerTest, 1);
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					auto x = LooperImpl::GetTestState();
					if (x == eTestState_0)
					{
						mHandler = mWeakHandler.lock();
						DV("mHandler=%p", mHandler.get());
						if (mHandler.get() == nullptr)
						{
							DW("error");
						}

						LooperImpl::SetTestState(eTestState_1);
					}
					else if (x == eTestState_2)
					{
						mHandler = nullptr;
						LooperImpl::SetTestState(eTestState_3);
					}

				}

				__super::OnTimer(id);
			}

			weak_ptr<Handler>	mWeakHandler;
			shared_ptr<Handler> mHandler;
			long mTimerTest = 0;

		};

		auto looper = make_shared<MainLooper>();
		auto ret = looper->StartRun();
		DW("%s#1", __func__);
	}
#endif

	//XiongWanPing 2018.06.28
	//CrossLooperDestroyHandler的特点是跨looper析构handler
	//测试跨looper释放handler最后一个ref,看handler析构是否正常
	//测试办法:
	//.在WorkLooper中创建WorkHandler
	//.在MainLooper中对WorkHandler保持一个跨looper引用
	//.WorkHandler.Destroy(),并WorkLooper.PostQuitMessage
	//.延时在MainLooper中清空WorkHandler,这样对app来说就是在MainLooper析构最后释放WorkLooper中的WorkHandler
	TEST_METHOD(CrossLooperDestroyHandler)
	{
		enum
		{
			BM_TEST,
		};
		class WorkHandler :public Handler
		{
		public:
			WorkHandler()
			{
				DV("%s", __func__);
			}
			~WorkHandler()
			{
				DV("%s", __func__);
			}

			void OnCreate()
			{
				__super::OnCreate();
				mTimerTest = SetTimer(100);
			}

			void OnDestroy()
			{
				DW("%s", __func__);
				__super::OnDestroy();
			}
			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				switch (msg)
				{
				case BM_TEST:
				{
					static int idx = -1;
					++idx;
					DV("BM_TEST,idx=%04d", idx);
					return 0;
				}
				}

				return __super::OnMessage(msg, wp, lp);
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					DV("%s", __func__);
					++mValue;
					return;
				}

				__super::OnTimer(id);
			}

			long mTimerTest = 0;
			long mValue = 0;
		};

		enum
		{
			BM_GET_WORK_HANDLER,
		};
		class WorkLooper :public Looper
		{
		public:
			WorkLooper()
			{
				mThreadName = "WorkLooper";
			}
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<WorkHandler>();
				mWorkHandler = obj;
				AddChild(obj);
			}

			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				switch (msg)
				{
				case BM_GET_WORK_HANDLER:
				{
					auto handler = mWorkHandler.lock();
					if (handler)
					{
						return (LRESULT)handler.get();
					}
					return 0;
				}
				}

				return __super::OnMessage(msg, wp, lp);
			}

			weak_ptr<Handler> mWorkHandler;
		};

		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mThreadName = "MainLooper";
			}
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				auto looper = make_shared<WorkLooper>();
				AddChild(looper);
				auto ret = looper->Start();

				Handler *ptr = (Handler *)(looper->sendMessage(BM_GET_WORK_HANDLER));
				if (ptr)
				{
					mHandler = dynamic_pointer_cast<WorkHandler>(ptr->shared_from_this());
				}

				DW("WorkLooper PostQuitMessage");
				looper->PostQuitMessage();

				mTimerDelay = SetTimer(3000);
				mTimerTest = SetTimer(500);
				DT("mTimerDelay=%d,mTimerTest=%d", mTimerDelay, mTimerTest);
			}

			void OnTimer(long id)
			{
				DV("%s,id=%d", __func__, id);
				if (id == 1)
				{
					int x = 0;
				}

				if (id == mTimerTest)
				{
					if (mHandler)
					{
						mHandler->sendMessage(BM_TEST);
					}
				}
				else if (id == mTimerDelay)
				{
					mHandler = nullptr;
					KillTimer(mTimerDelay);

					DW("%s,PostQuitMessage", mThreadName.c_str());
					PostQuitMessage();
				}

				__super::OnTimer(id);
			}

			shared_ptr<Handler> mHandler;
			long mTimerDelay = 0;
			long mTimerTest = 0;
		};

		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(317);

		//auto event = make_shared<Event>(true,false);

		{
			auto looper = make_shared<MainLooper>();
			//looper->SetExitEvent(event);
			auto ret = looper->StartRun();
		}

		//event->Wait();
		//ShellTool::Sleep(2000);
	}

	//说明:经测试，跨looper创建handler会引起一些问题，并且此应用场景不是必须的，可用其他方法回避，所以决定不支持这种用法
	TEST_METHOD(CrossLooperCreateHandler)
	{
		class TestLooper :public Looper
		{
		public:
			TestLooper()
			{
				mObjectName = "TestLooper";
				mThreadName = mObjectName;
			}
		protected:
			int InitInstance()
			{
				DW("%s,this=%p", __func__, this);
				return __super::InitInstance();
			}
		};

		class TestHandler :public Handler
		{
		public:
			TestHandler()
			{
				DW("%s,this=%p", __func__, this);
			}
			~TestHandler()
			{
				DW("%s,this=%p", __func__, this);
			}
		protected:
			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				DV("%s", __func__);
				return __super::OnMessage(msg, wp, lp);
			}
			void OnCreate()
			{
				__super::OnCreate();

				DV("%s", __func__);

				class TestRunnable :public Runnable
				{
					void Run()
					{
						DW("%s", __func__);
						Looper::GetMainLooper()->PostQuitMessage();
					}
				};
				postRunnable(make_shared<TestRunnable>());
			}

			void OnDestroy()
			{
				DV("%s", __func__);
				__super::OnDestroy();
			}
		};

		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mObjectName = "MainLooper";
				mThreadName = mObjectName;
				SetMainLooper(this);
			}

		protected:
			int InitInstance()
			{
				{
					auto looper = make_shared<TestLooper>();
					AddChild(looper);
					looper->Start();

					auto obj = make_shared<TestHandler>();
					//looper->PostQuitMessage();

					//ShellTool::Sleep(1000);

					//obj->sendMessage(BM_NULL);
					looper->AddChild(obj);
					obj->Destroy();
					//obj->Destroy();
					
					Looper::GetMainLooper()->PostQuitMessage();
				}

				return __super::InitInstance();
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 0);
	}

	TEST_METHOD(TestCpp)
	{


		class Base
		{
			int mX = 0;
		};

		auto obj = make_shared<Base>();
		auto obj2 = make_shared<Base>();
		auto sp = obj;
		auto sp2 = obj2;
		sp = sp2;

		if (sp == sp2)
		{
			DV("is equal");
		}
		else
		{
			DW("is NOT equal");
		}
	}

	TEST_METHOD(NormalCase)
	{
		{
			//确保构造后能直接析构
			auto handler = make_shared<Handler>();
			auto looper = make_shared<Looper>();
		}
	}

	//显示CoreLooper典型用法
	TEST_METHOD(DemoUsage)
	{
		class WorkHandler :public Handler
		{
		public:
			WorkHandler()
			{
				DV("%s,this=%p", __func__, this);
			}

			~WorkHandler()
			{
				DV("%s,this=%p", __func__, this);
			}

			void SetTestTimer()
			{
				mTimerTest = SetTimer(300);
			}

		protected:
			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					static int idx = -1;
					++idx;
					DV("%s,idx=%d", __func__, idx);

					return;
				}
				__super::OnTimer(id);
			}

			long mTimerTest = 0;
		};
		class MainLooper :public Looper
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				{
					auto obj = make_shared<WorkHandler>();
					obj->Create(shared_from_this());
					mHandler = obj;
				}

				AddChild(make_shared<WorkHandler>());

				mTimerTest = SetTimer(1000);
				mTimerDelayQuit = SetTimer(5 * 1000);
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					KillTimer(mTimerTest);

					auto obj = mHandler.lock();
					if (obj)
					{
						DV("destroy %p", obj.get());
						obj->Destroy();

						//测试handler destroy之后timer是否能正常工作
						obj->SetTestTimer();

						mHandlerRef = obj;
						mTimerDelayRelease = SetTimer(1500);
					}
				}
				else if (id == mTimerDelayRelease)
				{
					KillTimer(mTimerDelayRelease);

					DV("release mHandlerRef");
					mHandlerRef = nullptr;
				}
				else if (id == mTimerDelayQuit)
				{
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

			long mTimerDelayQuit = 0;
			long mTimerTest = 0;
			long mTimerDelayRelease = 0;

			weak_ptr<WorkHandler> mHandler;
			shared_ptr<Handler> mHandlerRef;
		};

		{
			auto looper = make_shared<MainLooper>();
			auto ret = looper->StartRun();
		}
	}
};

}


namespace TestCore
{

TEST_CLASS(Runnable_UnitTest)
{
public:
	TEST_METHOD(Runnable_Demo)
	{
		{
			class MainLooper :public Looper
			{
				void OnCreate()
				{
					__super::OnCreate();

					class DelayExit :public Runnable
					{
					public:
						DelayExit()
						{
							DW("%s", __func__);
						}

						virtual ~DelayExit()
						{
							DW("%s", __func__);
						}

						void Run()
						{
							CurrentLooper()->PostQuitMessage();
						}
					};

					DV("postDelayedRunnable");
					postDelayedRunnable(make_shared<DelayExit>(), 1000);
				}
			};
			auto looper = make_shared<MainLooper>();
			looper->StartRun();
		}
	}

	TEST_METHOD(Runnable_sendRunnable)
	{
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mThreadName = "MainLooper";
			}

		protected:
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimerDelayExit, 5000);
			}

			void OnTimer(long id)
			{
				if (id == mTimerDelayExit)
				{
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

			long mTimerDelayExit = 0;
		};

		class WifiHelper :public Runnable
		{
		public:
			void Run()
			{
				DW("%s", __func__);
				static int idx = -1;
				++idx;

				if (mBuddy)
				{
					Looper::CurrentLooper()->cancelRunnable(mBuddy);
				}
			}

			shared_ptr<WifiHelper> mBuddy;
		};

		auto tlsLooper = Looper::BindTLSLooper();
		auto event = make_shared<Event>();
		{
			auto looper = make_shared<MainLooper>();
			looper->SetExitEvent(event);
			auto ret = looper->Start();

			auto r0 = make_shared<WifiHelper>();
			auto r1 = make_shared<WifiHelper>();
			auto r2 = make_shared<WifiHelper>();

			r0->mBuddy = r2;
			looper->sendRunnable(r0);
			looper->postDelayedRunnable(r1, 1000);
			looper->postDelayedRunnable(r2, 2000);
		}

		event->Wait();
	}

	//测试Runnable性能
	TEST_METHOD(Runnable_Performance)
	{
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mThreadName = "MainLooper";
			}
		};

		static long gTestTimes = 1000 * 1000;
		static long mTimes = 0;
		class WifiHelper :public Runnable
		{
		public:
			void Run()
			{
				if (++mTimes == gTestTimes)
				{
					Looper::CurrentLooper()->PostQuitMessage();
				}
			}

		};

		LONGLONG tick = 0;
		auto tlsLooper = Looper::BindTLSLooper();
		auto event = make_shared<Event>();
		{
			auto looper = make_shared<MainLooper>();
			looper->SetExitEvent(event);
			auto ret = looper->Start();

			auto obj = make_shared<WifiHelper>();
			tick = ShellTool::GetTickCount64();
			for (int i = 0; i < gTestTimes; i++)
			{
				looper->postRunnable(obj);
				//looper->postDelayedRunnable(obj,1);
			}
		}

		event->Wait();
		tick = ShellTool::GetTickCount64() - tick;
		DV("tick = %I64d", tick);//release版100万次约1.2秒,debug版约9.2秒
		if (tick > 0)
		{
			long speed = (long)(gTestTimes *1000.0 / tick);
			DV("speed = %d/s", (int)speed);//release版88万/秒,debug版10万/秒
		}
	}

	TEST_METHOD(Runnable_CancelOtherLooper)
	{
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mThreadName = "MainLooper";
			}

		protected:
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimerDelayExit, 1000);
			}

			void OnTimer(long id)
			{
				if (id == mTimerDelayExit)
				{
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

			long mTimerDelayExit = 0;
		};

		class WifiHelper :public Runnable
		{
		public:
			void Run()
			{
				mExecuted = true;

				static int idx = -1;
				++idx;
				DW("%s,idx=%04d", __func__, idx);
			}

			bool IsExecuted()const
			{
				return mExecuted;
			}
		protected:
			volatile bool mExecuted = false;
		};

		auto tlsLooper = Looper::BindTLSLooper();
		auto event = make_shared<Event>();
		auto obj = make_shared<WifiHelper>();
		{
			auto looper = make_shared<MainLooper>();
			looper->SetExitEvent(event);
			auto ret = looper->Start();
			looper->sendMessage(BM_NULL);

			looper->postRunnable(obj);
			//looper->postDelayedRunnable(obj,1000);
			looper->cancelRunnable(obj);
		}
		event->Wait();

		DV("obj->IsExecuted()=%d", obj->IsExecuted());
		Assert::AreEqual(obj->IsExecuted(), true);
	}

	TEST_METHOD(Runnable_CancelCurrentLooper)
	{
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				SetMainLooper(this);
				mThreadName = "MainLooper";
			}

		protected:
			class WifiHelper :public Runnable
			{
			public:
				void Run()
				{
					mExecuted = true;

					static int idx = -1;
					++idx;
					DW("%s,idx=%04d", __func__, idx);
				}

				bool IsExecuted()const
				{
					return mExecuted;
				}
			protected:
				volatile bool mExecuted = false;
			};

			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimerDelayExit, 1000);


				auto obj = make_shared<WifiHelper>();
				mWifiHelper = obj;
				postRunnable(obj);
				postRunnable(obj);
				cancelRunnable(obj);
			}

			void OnTimer(long id)
			{
				if (id == mTimerDelayExit)
				{
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

			long mTimerDelayExit = 0;
		public:
			shared_ptr<WifiHelper> mWifiHelper;
		};

		{
			auto looper = make_shared<MainLooper>();
			looper->StartRun();
			Assert::IsFalse(looper->mWifiHelper->IsExecuted());
		}
	}

	TEST_METHOD(TestPostRunnable)
	{
		class MainLooper :public Looper
		{
		public:
			MainLooper()
			{
				mObjectName = "MainLooper";
				mThreadName = mObjectName;

				SetMainLooper(this);
			}

		protected:
			int InitInstance()
			{
				class WorkLooper :public Looper
				{
				public:
					WorkLooper()
					{
						DV("%s,this=%p", __func__, this);
					}
					~WorkLooper()
					{
						DV("%s,this=%p", __func__, this);
					}
				};
				class Worker :public Runnable
				{
				public:
					Worker()
					{
						DV("%s,this=%p", __func__, this);
					}
					virtual ~Worker()
					{
						DV("%s,this=%p", __func__, this);
					}
					void Run()
					{
						Looper::GetMainLooper()->PostQuitMessage();
					}
				};

				auto looper = make_shared<WorkLooper>();
				AddChild(looper);
				looper->Start();
				looper->postRunnable(make_shared<Worker>());

				return __super::InitInstance();
			}
		};

		{
			auto obj = make_shared<MainLooper>();
			auto ret = obj->StartRun();
		}

	}
};

TEST_CLASS(Timer_UnitTest)
{
public:
	//2018.07.27,为统一起见，不再支持在Handler构造函数中SetTimer和KillTimer
	//必须在触发OnCreate之后才能调用SetTimer
	TEST_METHOD(Timer_SetTimerKillTimer_HandlerConstruction)
	{
		class MainLooper :public Looper
		{
			void OnCreate()
			{
				__super::OnCreate();

				class WifiManager :public Handler
				{
				public:
					WifiManager()
					{
					}
					void OnCreate()
					{
						SetTimer(mTimerDelayExit, 1000);
						DV("set timer in construction,id=%d", mTimerDelayExit);
						DV("kill timer in construction#before,id=%d", mTimerDelayExit);
						KillTimer(mTimerDelayExit);
						DV("kill timer in construction#after,id=%d", mTimerDelayExit);

						SetTimer(mTimerDelayExit, 3000);
						DV("re-set timer in construction,id=%d", mTimerDelayExit);
						__super::OnCreate();
					}

				protected:
					void OnTimer(long id)
					{
						if (id == mTimerDelayExit)
						{
							DV("%s,id=%d", __func__, id);
							KillTimer(mTimerDelayExit);
							Looper::CurrentLooper()->PostQuitMessage();

							return;
						}

						__super::OnTimer(id);
					}

					long mTimerDelayExit = 0;
				};

				AddChild(make_shared<WifiManager>());
			}
		};

		{
			auto looper = make_shared<MainLooper>();
			looper->StartRun();
		}
	}

	//注意:不支持在Looper子类的构造函数中直接SetTimer/KillTimer,必须start looper后才能调用
	//TEST_METHOD(Timer_SetTimerKillTimer_LooperConstruction)
	//{
	//}
};

}

//直接namespace Bear::Net::Http{{{
//vs会报error C2429: language feature 'nested-namespace-definition' requires compiler flag '/std:c++latest'
namespace Bear {
namespace Net {
namespace Http {
class HttpClient
{
public:
	HttpClient()
	{
		DV("%s", __func__);
	}
};
}}}


BEGIN_TEST_MODULE_ATTRIBUTE()
TEST_MODULE_ATTRIBUTE(L"Date", L"2010/6/12")
END_TEST_MODULE_ATTRIBUTE()

TEST_MODULE_INITIALIZE(ModuleInitialize)
{
	Logger::WriteMessage("In Module Initialize");
}

TEST_MODULE_CLEANUP(ModuleCleanup)
{
	Logger::WriteMessage("In Module Cleanup");
}

TEST_CLASS(TestClass1)
{

public:

	TestClass1()
	{
		Logger::WriteMessage("In TestClass1");
		DV("%s", __func__);
	}

	~TestClass1()
	{
		Logger::WriteMessage("In ~TestClass1");
		DV("%s", __func__);
	}

	TEST_CLASS_INITIALIZE(ClassInitialize)
	{
		Logger::WriteMessage("In Class Initialize");
	}

	TEST_CLASS_CLEANUP(ClassCleanup)
	{
		Logger::WriteMessage("In Class Cleanup");
	}

	BEGIN_TEST_METHOD_ATTRIBUTE(Method1)
		TEST_OWNER(L"OwnerName")
		TEST_PRIORITY(4)
	END_TEST_METHOD_ATTRIBUTE()

	TEST_METHOD(Method1)
	{
		Logger::WriteMessage("In Method1");
		Assert::AreEqual(0, 0);
	}

	TEST_METHOD(Method2)
	{
		DV("%s", __func__);
		//Assert::Fail(L"Fail");
	}

	BEGIN_TEST_METHOD_ATTRIBUTE(Method3)
		TEST_OWNER(L"OwnerName")
		TEST_PRIORITY(1)
	END_TEST_METHOD_ATTRIBUTE()
	
	TEST_METHOD(Method3)
	{
		Logger::WriteMessage("In Method3");
		Assert::AreEqual(0, 0);
	}

};

#define _CONFIG_NLOHMANN_JSON
#ifdef _CONFIG_NLOHMANN_JSON
#include "d:\\os\\json\\nlohmann\\single_include\\nlohmann\\json.hpp"

using json = nlohmann::json;

TEST_CLASS(NlohmannJson)
{
	//https://github.com/nlohmann/json
	//这个网页有各种用法demo
	string mJsonText;

	TEST_METHOD(Create)
	{
		{
			json o;
			auto& j = o;
			o["age"] = 23;
			o["bar"] = false;
			o["baz"] = 3.141;
			o["list"] = { 2018, 01, 06 };
			o["title"] = "This is a very long text,\'he\"\"llo\r\n line2?";
			o["answer"]["everything"] = 42;
			File::Dump(o.dump(4), "d:/t.json");//dump(4)是pretty格式，indent为4个空格,效果很好!
			DV("[%s]", o.dump(4).c_str());
			DV("name=[%d]", o["age"].get<int32_t>());
			DV("title=[%s]", o["title"].get<string>().c_str());

			mJsonText = o.dump();
		}

		{
			auto o = json::parse(mJsonText, nullptr, false);
			
			
			std::string s = o.dump();
			DV("[%s]", o["title"].get<string>().c_str());
			//o.emplace_back()
			//DV("type=%s", o.at("list").type.get<string>());
			o.size();
			
			//用o.at()时如果name不存在会crash,用o.[]时返回空值，不会crash
			auto& items = o["list"];
			auto nc = items.size();//items为空值时.size()会返回0,不会crash
			DV("items nc=%d", nc);
			for (int i = 0; i < nc; i++)
			{
				DV("item[%d]=[%d]", i, items.at(i).get<int>());//json会记住类型，类型要匹配，否则报crash
			}
		}
		
	}
};

TEST_CLASS(Enjoy)
{

	//地球总人口体积
	TEST_METHOD(HumanM3)
	{
		ULONGLONG count = 1000 * 1000 * 1000 * (ULONGLONG)8;//80亿
		ULONGLONG averageKG = 50;//平均每人50千克
								 //double density = 1.06;//比水的密度大那么一点点
		double personalVolume = averageKG / 1000.0;//按水的密度来算
		DV("personalVolume=%.2f m3", personalVolume);
		auto totalVolume = (ULONGLONG)(count * personalVolume);
		DV("totalVolume=%lld m3", totalVolume);

		//立方千米
		totalVolume /= 1000;
		totalVolume /= 1000;
		DV("height=%lld (km2)", totalVolume);
		//1000米*1000米的正方形，堆400米高，即0.4立方千米
	}

	TEST_METHOD(TestDT)
	{
		DV("Hello,x64");
		DW("Hello,x64");
		DE("Hello,x64");
		DT("done");
	}

};

#endif



