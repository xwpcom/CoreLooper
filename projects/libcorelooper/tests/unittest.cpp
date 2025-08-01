#include "stdafx.h"
#include "unittest.h"
#include "CppUnitTest.h"
#include "core/looper/teststate.h"
#include "core/base/filelogger.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "net/udpserver.h"
#include "arch/windows/udpclient_windows.h"
#include <cmath>


#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

/*
XiongWanPing 2016~
本单元测试用来测试各种场景，包括正常场景和故意构造的极端调用场景,
以此来保证CoreLooper框架在各种调用场景的稳定性
.每个测试必须能快速进行，以便快速迭代
*/
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;
static const char* TAG = "Core";

namespace CPP11_Test
{


TEST_CLASS(_TestCPP11)
	{
	public:
		TEST_METHOD(_bind)
		{
			class DemoLooper:public Looper
			{
			public:
				DemoLooper()
				{
					SetObjectName("DemoLooper");
				}
			};

			class MainLooper :public MainLooper_
			{
				void OnCreate()
				{
					__super::OnCreate();

					{
						LogV(TAG,"main threadId=%d", ShellTool::GetCurrentThreadId());

						auto obj = make_shared<DemoLooper>();
						AddChild(obj);
						obj->Start();

						string name = "xwp";
						auto func = [=, &name](int a, int b, float c)
						{
							Assert::IsTrue(obj->IsMyselfThread());

							LogV(TAG,"lambda,a=%d,b=%d,c=%.1f", a, b, c);
							LogV(TAG,"lambda threadId=%d,obj.name=%s", ShellTool::GetCurrentThreadId(),obj->GetObjectName().c_str());
							name = "bear";
							Looper::GetMainLooper()->PostQuitMessage();
						};

						obj->sendRunnable(std::bind(func, 100, 'c', 2.5f));
						LogV(TAG,"name=%s", name.c_str());
					}
				}

			};

			make_shared<MainLooper>()->StartRun();
		}

		TEST_METHOD(lambda_defer_call)
		{
			class Worker:public Handler
			{
				string mTag = "worker";
				vector<TaskEntry> mPendingTasks;//等待在connected之后执行的任务
			public:
				void doCall()
				{
					for (auto& t : mPendingTasks)
					{
						t();
					}

					mPendingTasks.clear();
				}

				void ReportFlow(const string& value)
				{
					LogV(mTag, "%s(%s)", __func__, value.c_str());

				}

				void addDeferCall(const string& value)
				{
					mPendingTasks.push_back([this, value]() {
						ReportFlow(value);
						});
				}
			};

			class MainLooper :public MainLooper_
			{
				void OnCreate()
				{
					__super::OnCreate();

					auto obj = make_shared<Worker>();
					AddChild(obj);

					{
						string value = "12";
						obj->addDeferCall(value);
					}
					{
						string value = "34";
						obj->addDeferCall(value);
					}

					obj->doCall();

					DelayExit(3000);

				}
			};

			make_shared<MainLooper>()->StartRun();
		}

		TEST_METHOD(sortBase)
		{
			//https://stackoverflow.com/questions/11357918/how-to-sort-a-container-of-stdshared-ptrwidget-objects
			class Widget
			{
			public:
				Widget(const string& n)
				{
					name = n;
				}
				string name;
			};
			class Criterium
			{
			public:
				bool operator()(const Widget& left, const Widget& right)const
				{
					return left.name < right.name;
				}
			};

			std::vector< std::shared_ptr<Widget> > items;
			items.push_back(make_shared<Widget>("apple"));
			items.push_back(make_shared<Widget>("xwp"));
			items.push_back(make_shared<Widget>("bear"));

			Criterium criterium;
			
			if (1)
			{
				sort(items.begin(), items.end(),
					 [](const shared_ptr<Widget>& l, const shared_ptr<Widget>& r)
					 {
						 return l->name < r->name;
					 }
				);
			}
			else
			{
				sort(items.begin(), items.end(),
					 [&criterium](const shared_ptr<Widget>& l, const shared_ptr<Widget>& r)
					 {
						 return criterium(*l.get(), *r.get());
					 }
				);
			}

			for (auto& item : items)
			{
				LogV(TAG, "%s",item->name.c_str());
			}



		}

		TEST_METHOD(_lambda_base)
		{

			// Create a vector object that contains 10 elements.
			vector<int> v;
			for (int i = 1; i < 10; ++i) {
				v.push_back(i);
			}

			std::sort(v.begin(), v.end(), 
				[](int a, int b) {
					return a < b;
				}
				);
			
			{
				string name = "bear";
				auto obj = [&name](int first, int second)
				{
					return first + second;
				};

				obj(12, 34);

				function<LRESULT (shared_ptr<Handler>,int, int)> fn;
			}

			// Count the number of even numbers in the vector by
			// using the for_each function and a lambda.
			int evenCount = 0;
			for_each(v.begin(), v.end(), [&evenCount](int n) {
				cout << n;
				if (n % 2 == 0) {
					cout << " is even " << endl;
					++evenCount;
				}
				else {
					cout << " is odd " << endl;
				}
				});
			vector<string> v2;
			v2.push_back("hello");
			for_each(v2.begin(), v2.end(), [&evenCount](string& n) {
				n = "world";
				});

			LogV(TAG,"item=%s", v2[0].c_str());

			// Print the count of even numbers to the console.
			cout << "There are " << evenCount
				<< " even numbers in the vector." << endl;
		}
		TEST_METHOD(_lambda_)
		{
			class LambdaTest
			{
			public:
				static void abssort(float* x, unsigned n) {
					int i = 0;
					std::sort(x, x + n,
						// Lambda expression begins
						[](float a, float b) {
							return (std::abs(a) < std::abs(b));
						} // end of lambda expression
					);
				}
			};

			{
				float arr[] = { 1,2,3 };
				LambdaTest::abssort(arr, COUNT_OF(arr));
			}
		}

		TEST_METHOD(Test_typeid)
		{
			class TestHandler :public Handler
			{

			};
			LogV(TAG,"name=%s",typeid(TestHandler).name());
			LogV(TAG,"raw_name=%s", typeid(TestHandler).raw_name());
		}

		//学习std move
		//https://docs.microsoft.com/en-us/cpp/cpp/move-constructors-and-move-assignment-operators-cpp?view=vs-2017
		TEST_METHOD(StdMove)
		{
			class MemoryBlock
			{
			public:
				explicit MemoryBlock(size_t length)
					: _length(length)
					, _data(new int[length])
				{
					LogV(TAG,"%s(%p),length = %d",__func__, this,_length);
				}

				// Destructor.
				~MemoryBlock()
				{
					LogV(TAG,"%s(%p),length = %d", __func__, this, _length);

					if (_data != nullptr)
					{
						//LogV(TAG," Deleting resource.");
						delete[] _data;
					}

					std::cout << std::endl;
				}

				// Copy constructor.
				MemoryBlock(const MemoryBlock& other)
					: _length(other._length)
					, _data(new int[other._length])
				{
					LogV(TAG,"%s(%p)#copy,length = %d", __func__, this, _length);
					//LogV(TAG,"(%p)In MemoryBlock(const MemoryBlock&). length = %d . Copying resource."
						//, this
						//, other._length);

					std::copy(other._data, other._data + _length, _data);
				}

				// Copy assignment operator.
				MemoryBlock& operator=(const MemoryBlock& other)
				{
					LogV(TAG,"%s(%p)#=,length = %d", __func__, this, _length);
					//LogV(TAG,"(%p)In operator=(const MemoryBlock&). length = %d,Copying resource."
						//, this
						//, other._length);

					if (this != &other)
					{
						// Free the existing resource.
						delete[] _data;

						_length = other._length;
						_data = new int[_length];
						std::copy(other._data, other._data + _length, _data);
					}
					return *this;
				}

				// Retrieves the length of the data resource.
				size_t Length() const
				{
					return _length;
				}

				MemoryBlock(MemoryBlock&& other)
					: _data(nullptr)
					, _length(0)
				{
					LogV(TAG,"%s(%p)#move,length = %d", __func__, this, _length);
					// Copy the data pointer and its length from the
					// source object.
					_data = other._data;
					_length = other._length;

					// Release the data pointer from the source object so that
					// the destructor does not free the memory multiple times.
					other._data = nullptr;
					other._length = 0;
				}

				// Move assignment operator.
				MemoryBlock& operator=(MemoryBlock&& other)
				{
					LogV(TAG,"%s(%p)#move=,length = %d", __func__, this, _length);

					if (this != &other)
					{
						// Free the existing resource.
						delete[] _data;

						// Copy the data pointer and its length from the
						// source object.
						_data = other._data;
						_length = other._length;

						// Release the data pointer from the source object so that
						// the destructor does not free the memory multiple times.
						other._data = nullptr;
						other._length = 0;
					}
					return *this;
				}

			private:
				size_t _length; // The length of the resource.
				int* _data; // The resource.
			};


			if (0)
			{
				std::string str = "Hello";
				str = std::move(str);
				std::string str2 = std::move(str);
				
				LogV(TAG,"str=[%s]", str.c_str());
				LogV(TAG,"str2=[%s]", str2.c_str());
			}
			else if (1)
			{
				class Person
				{
				public:
					Person()
					{
						LogV(TAG,"%s(%p)", __func__, this);
					}
					
					Person(const Person& src)
					{
						LogV(TAG,"%s(%p)#copy", __func__, this);
					}
					Person(Person&& src)
					{
						LogV(TAG,"%s(%p)", __func__, this);

						*this = std::move(src);

						//mName = src.mName;
						//mBirthday = src.mBirthday;

					}

					Person& operator=(Person&& src)
					{
						if (this != &src)
						{
							//todo
						}
						return *this;
					}
					virtual ~Person()
					{
						LogV(TAG,"%s(%p)", __func__, this);
					}
				protected:
					string mName;
					int mBirthday;
				};

				vector<Person> arr;

				Person obj1;
				arr.push_back(obj1);
			}
			else
			{
				vector<MemoryBlock> v;
				v.push_back(MemoryBlock(25));
				//v.push_back(MemoryBlock(75));

				// Insert a new element into the second position of the vector.
				//v.insert(v.begin() + 1, MemoryBlock(50));
			}
		}
	};
}

namespace UnitTest
{
TEST_CLASS(Sigslot_)
{
public:
	TEST_METHOD(Base)
	{
		class EventSource :public Handler
		{
			SUPER(Handler)
		public:
			sigslot::signal3<Handler*, const string&, int> SignalSomeEvent;

		protected:
			void OnCreate()
			{
				__super::OnCreate();

				class Worker :public Runnable
				{
				public:
					weak_ptr<EventSource> mObject;
				protected:
					void Run()
					{
						auto obj = mObject.lock();
						if (obj)
						{
							obj->SignalSomeEvent(obj.get(), "hello", 2019);
						}
					}
				};

				auto obj = make_shared<Worker>();
				obj->mObject = dynamic_pointer_cast<EventSource>(shared_from_this());
				postDelayedRunnable(obj, 1000);
			}

		};
		class EventListener :public Handler
		{
			SUPER(Handler)
		public:
			void OnSomeEvent(Handler*, const string& msg, int value)
			{
				DW("%s,msg=%s,value=%d", __func__, msg.c_str(), value);

				Looper::CurrentLooper()->PostQuitMessage(0);
			}
		};

		class MainLooper :public MainLooper_
		{
			SUPER(MainLooper_);

			void OnCreate()
			{
				__super::OnCreate();

				auto source = make_shared<EventSource>();
				AddChild(source);

				auto listener = make_shared<EventListener>();
				AddChild(listener);

				source->SignalSomeEvent.connect(listener.get(), &EventListener::OnSomeEvent);
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

};

TEST_CLASS(Handler_)
{
public:
	TEST_METHOD(testPost)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				class Worker :public Handler
				{
				public:
					void func()
					{
						LogV(TAG, "%s",__func__);
						post([&]() {
							delayApi(123);
							},5000);
					}

					void delayApi(int v)
					{
						LogV(TAG, "%s", __func__);
					}
				};

				auto obj = make_shared<Worker>();
				AddChild(obj);
				obj->func();
				obj->Destroy();

				DelayExit(3000);

			}
		};

		make_shared<MainLooper>()->StartRun();

	}
	TEST_METHOD(TestCopyFile)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				class CopyFileTask :public AsyncTask
				{
				public:
					CopyFileTask()
					{
						LogV(TAG,"%s", __func__);
					}
					~CopyFileTask()
					{
						LogV(TAG,"%s", __func__);
					}

					string mSourceFilePath;
					string mDestFilePath;
				protected:
					void Run()
					{
						auto ret = File::CopyFile(mSourceFilePath, mDestFilePath);
						LogV(TAG,"FileEx::CopyFile ret=%d", ret);
					}
					void OnPostExecute()
					{
						LogV(TAG,"%s", __func__);
						Looper::CurrentLooper()->PostQuitMessage();
					}
				};

				LogV(TAG,"start copy file");
				auto obj = make_shared<CopyFileTask>();
				//obj->mWorker = dynamic_pointer_cast<HuiZhouWorker>(shared_from_this());
				obj->mSourceFilePath = "d:/t.cpp";
				obj->mDestFilePath = "d:/t.copy.cpp";
				obj->Execute();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	//测试android jni下需要用到的场景
	TEST_METHOD(JniTest)
	{
		class AHandler :public Handler
		{
			void OnCreate()
			{
				__super::OnCreate();
				DG("%s,threadId=%d", __func__,ShellTool::GetCurrentThreadId());
			}
		};

		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
			}
		};

		auto looper = make_shared<MainLooper>();
		looper->Start();
		looper->sendMessage(BM_NULL);

		auto obj = make_shared<AHandler>();
		looper->AddChild(obj);

		while (1)
		{
			ShellTool::Sleep(1);
			break;
		}
	}

	TEST_METHOD(DemoAndroidActivity)
	{

		class Activity :public Handler
		{
		public:
			const UINT mMessageStart = BindMessage(&Activity::OnStart);
			const UINT mMessageResume = BindMessage(&Activity::OnResume);
			const UINT mMessagePause = BindMessage(&Activity::OnPause);
			const UINT mMessageStop = BindMessage(&Activity::OnStop);
			const UINT mMessageRestart = BindMessage(&Activity::OnRestart);
			const UINT mMessageTest = BindMessage(&Activity::OnTest);
		protected:

			virtual LRESULT OnTest(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}

			virtual void OnCreate()
			{
				__super::OnCreate();

				LogV(TAG,"%s", __func__);
			}

			virtual LRESULT OnStart(WPARAM wp,LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}

			virtual LRESULT OnResume(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}

			virtual LRESULT OnPause(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}
			
			virtual LRESULT OnStop(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}

			virtual LRESULT OnRestart(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return 0;
			}

			virtual void OnDestroy()
			{
				__super::OnDestroy();

				LogV(TAG,"%s", __func__);
			}

		};

		class ActivityEx :public Activity
		{
			SUPER(Activity);

			LRESULT OnTest(WPARAM wp,LPARAM lp)
			{
				__super::OnTest(wp,lp);
				LogV(TAG,"%s", __func__);
				return 0;
			}
		};

		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<ActivityEx>();
				AddChild(obj);

				obj->sendMessage(obj->mMessageStart);
				obj->sendMessage(obj->mMessageResume);
				obj->sendMessage(obj->mMessagePause);
				obj->sendMessage(obj->mMessageStop);
				obj->sendMessage(obj->mMessageRestart);
				obj->sendMessage(obj->mMessageTest);

				PostQuitMessage();
			}
		};
		
		{
			auto obj = make_shared<MainLooper>();
			obj->StartRun();
			obj->GetQuitCode();
		}

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TestAndroidStyleMessage)
	{
		class MainLooper :public MainLooper_
		{

			void OnCreate()
			{
				__super::OnCreate();

				class TestHandler :public Handler
				{
					void OnCreate()
					{
						__super::OnCreate();

						{
							auto msg = make_shared<Message>();
							msg->what = 2017;
							sendMessage(msg);
						}

						{
							auto msg = make_shared<Message>();
							msg->what = 2018;
							msg->arg1 = 9;
							msg->arg2 = 23;
							postMessage(msg);
						}
					}

					void HandleMessage(shared_ptr<Message> msg)
					{
						LogV(TAG,"what=%d,arg1=%lld,arg2=%lld", msg->what,msg->arg1,msg->arg2);
						//auto p = new int;
						if (msg->what == 2018)
						{
							CurrentLooper()->PostQuitMessage();
						}
					}
				};

				AddChild(make_shared<TestHandler>());
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(_TestRegisterMessageHandler)
	{	
		class Handler2 :public Handler
		{
		public:
			const UINT mTestMessageId = AllocMessageId();
		protected:
			enum
			{
				BM_TEST,
			};


			void OnCreate()
			{
				__super::OnCreate();

				LogV(TAG,"mTestMessageId=%d", mTestMessageId);
				LogV(TAG,"alloc id#1=%d", AllocMessageId());
				LogV(TAG,"alloc id#2=%d", AllocMessageId());


				BindMessageEx(mTestMessageId, &Handler2::OnTestMessage);
#ifdef _M_X64
				postMessage(mTestMessageId,123456789012345678, 12345678901234567);
				sendMessage(mTestMessageId, 12345678901234567, 123456789012345678);
#endif

				GetCurrentLooper()->PostQuitMessage();
			}

			LRESULT OnTestMessage(WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s,wp=%lld,lp=%lld", __func__,wp,lp);
				return 0;
			}
		};
		class Handler3 :public Handler2
		{
			void OnCreate()
			{
				__super::OnCreate();

				LogV(TAG,"mTestMessageId3=%d", mTestMessageId3);
			}

			public:
			UINT mTestMessageId3 = AllocMessageId();

		};

		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				AddChild(make_shared<Handler3>());
			}
		};

		make_shared<MainLooper>()->StartRun();
	}
	
	TEST_METHOD(TestString)
	{
		CString text = _T("中文");
		USES_CONVERSION;
		auto data = T2A(text);
		int x = 0;
	}

	void TestBase(int a, int b)
	{
		int c, d, e;
		LogV(TAG,"&a=%p", &a);
		LogV(TAG,"&b=%p", &b);
		LogV(TAG,"&c=%p", &c);
		LogV(TAG,"&d=%p", &d);
		LogV(TAG,"&e=%p", &e);
	}

	TEST_METHOD(TestBase_)
	{
		//TestBase(1, 2);

		class Base
		{
		protected:
			string mTag = "base";
		public:
			virtual ~Base()
			{
				fun();
			}
			virtual void fun()
			{
				LogV(mTag, "%s",__func__);
			}
		};
		
		class Child:public Base
		{
		public:
			Child()
			{
				mTag = "child";
			}
			~Child()override
			{
				fun();
			}
			virtual void fun()
			{
				LogV(mTag, "%s", __func__);
			}
		};

		{
			Child obj;
		}
	}

	TEST_METHOD(TestShortcut)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				class SpiManager :public Handler
				{
				public:
					SpiManager()
					{
						SetObjectName("SpiManager");
					}
				protected:
					void OnCreate()
					{
						__super::OnCreate();

						GetMainLooper()->RegisterShortcut("SpiManager", shared_from_this());
					}
				};

				class WorkLooper :public Looper
				{
					void OnCreate()
					{
						__super::OnCreate();
						AddChild(make_shared<SpiManager>());
					}
				};

				{
					auto looper = make_shared<WorkLooper>();
					AddChild(looper);
					looper->Start();
					looper->sendMessage(BM_NULL);
				}

				class TestHandler2 :public TestHandler
				{
					void OnCreate()
					{
						__super::OnCreate();

						//shared_ptr<SpiManager> obj = ShortcutEx("SpiManager");
					}
				};

				{
					auto obj = dynamic_pointer_cast<SpiManager>(Shortcut("SpiManager"));

					if (obj)
					{
						LogV(TAG,"obj=%s", obj->GetObjectName().c_str());
					}
					else
					{
						DW("obj is null");
					}
				}

				{
					auto obj = _Shortcut(this, SpiManager, "SpiManager");

					if (obj)
					{
						LogV(TAG,"obj=%s", obj->GetObjectName().c_str());
					}
					else
					{
						DW("obj is null");
					}
				}


				PostQuitMessage();
			}

		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(CrossLooperCreateHandler)
	{
		class TestLooper :public Looper
		{
		public:
			TestLooper()
			{
				SetObjectName("TestLooper");
				mThreadName = GetObjectName();
			}
		protected:
		};

		class TestHandler :public Handler
		{
		public:
			TestHandler()
			{
				LogV(TAG,"%s,this=%p", __func__, this);
			}
			~TestHandler()
			{
				LogV(TAG,"%s,this=%p", __func__, this);
			}
		protected:
			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				LogV(TAG,"%s", __func__);
				return __super::OnMessage(msg, wp, lp);
			}
			void OnCreate()
			{
				__super::OnCreate();

				LogV(TAG,"%s", __func__);

				class TestRunnable :public Runnable
				{
					void Run()
					{
						LogV(TAG,"%s", __func__);
						Looper::GetMainLooper()->PostQuitMessage();
					}
				};
				postRunnable(make_shared<TestRunnable>());
			}

			void OnDestroy()
			{
				LogV(TAG,"%s", __func__);
				__super::OnDestroy();
			}
		};

		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

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
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 0);
	}

	TEST_METHOD(CreateCrossLooperHandlerEx)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto looper = make_shared<Looper>();
				looper->Start();
				AddChild(looper);

				enum
				{
					BM_TEST,
				};
				class Worker :public Handler
				{
				public:
					Worker()
					{
						SetObjectName("Worker");
						DG("Worker,this=%p", this);
					}
					~Worker()
					{
						DG("~Worker,this=%p", this);
					}
				protected:
					int mIndex = -1;
					void OnCreate()
					{
						__super::OnCreate();

						DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());

						SetTimer(mTimerTest, 200);
					}

					void OnTimer(long id)
					{
						if (id == mTimerTest)
						{
							DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());

							
							++mIndex;
							if (mIndex == 5)
							{
								Looper::GetMainLooper()->PostQuitMessage();
							}
						}

						__super::OnTimer(id);
					}

					LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
					{
						switch (msg)
						{
							case BM_TEST:
							{
								LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
								return 0;
							}
						}

						return __super::OnMessage(msg, wp, lp);
					}

					long mTimerTest = 0;
				};

				DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
				//auto obj = make_shared<Worker>(looper);
				//looper->AddChild(obj);

				//looper->PostQuitMessage();
				//ShellTool::Sleep(1000);

				{
					auto obj = make_shared<Worker>();
					if (obj)
					{
						LogV(TAG,"CreateLooperHandler ok");
						class TestRunnable :public Runnable
						{
							void Run()
							{
								LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
							}
						};

						auto r = make_shared<TestRunnable>();
						//obj->sendMessage(BM_TEST);
						//obj->Destroy();
						looper->AddChild(obj);
						obj->postRunnable(r);
						//obj->sendMessage(BM_NULL);

						{
							auto obj2 = make_shared<Worker>();
							obj->AddChild(obj2);
						}
					}
					else
					{
						DW("CreateLooperHandler fail");
					}
				}

				//obj->Destroy();

				//PostQuitMessage();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TestStdMutex)
	{
		std::mutex obj;
		CriticalSection cs;

		auto tick = ShellTool::GetTickCount64();

		int times = 1000 * 1000*10;

		for (int i = 0; i < times; i++)
		{
			obj.lock();
			obj.unlock();
			//cs.Lock();
			//cs.Unlock();
		}

		tick =ShellTool::GetTickCount64()-tick;
		LogV(TAG,"tick=%lld", tick);

	}
	TEST_METHOD(postAsync)
	{
		class WorkLooper :public BlockLooper
		{
		};

		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();
				LogV(TAG, "%s", __func__);

				auto obj = make_shared <WorkLooper>();
				AddChild(obj);
				obj->Start();

				string text = "Hello";
				auto fn = [text]()
				{
					LogI(TAG, "%s,text=[%s]", __func__, text.c_str());
				};
				obj->post(fn
					,10*1000
				);

				text = "#1";
				LogV(TAG, "#2text=[%s]", text.c_str());
				
				{
					/*
					测试场景:有post还没执行时，提前退出main looper
					*/
					obj->post([&]() {
							PostQuitMessage();
						}
						, 1 * 1000
					);
				}
			}

		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(SmartTlsLooper_)
	{
		
		class MainLooper :public MainLooper_
		{
			enum
			{
				BM_TEST=1,
			};

			static void* WINAPI _RawThreadCB(void* p)
			{
				int i = (int)(LONGLONG)p;

				//LogV(TAG,"%s,i=%d#begin", __func__, i);
				auto obj=Looper::GetMainLooper();
				for(int idx=0;idx<10;idx++)
				{
					obj->sendMessage(BM_TEST, (WPARAM)i);
					
					string name = "xwp";
					
					auto func2 = [=, &name](int a, int b, float c)
					{
						Assert::IsTrue(obj->IsMyselfThread());

						LogV(TAG,"lambda,a=%d,b=%d,c=%.1f", a, b, c);
						LogV(TAG,"lambda threadId=%d,obj.name=%s", ShellTool::GetCurrentThreadId(), obj->GetObjectName().c_str());
						name = "bear";
						//Looper::GetMainLooper()->PostQuitMessage();
					};


					auto func = [=, &name](int a, int b, float c)
					{
						Assert::IsTrue(obj->IsMyselfThread());

						LogV(TAG,"lambda,a=%d,b=%d,c=%.1f", a, b, c);
						LogV(TAG,"lambda threadId=%d,obj.name=%s", ShellTool::GetCurrentThreadId(), obj->GetObjectName().c_str());
						name = "bear";
						//Looper::GetMainLooper()->PostQuitMessage();

						obj->sendRunnable(std::bind(func2, 100, 'c', 2.5f));
					};

					obj->sendRunnable(std::bind(func, 100, 'c', 2.5f));
				}
				//LogV(TAG,"%s,i=%d#end", __func__, i);
				return nullptr;
			}

			class WorkLooper :public Looper
			{
				int mTimes = 10;
				void OnCreate()
				{
					__super::OnCreate();

					for (int i = 0; i < mTimes; i++)
					{
						LONGLONG index = i;
						ShellTool::QueueUserWorkItem((LPTHREAD_START_ROUTINE)_RawThreadCB, (void*)index);

						ShellTool::Sleep(10);
					}
				}

			};

			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<WorkLooper>();
				AddChild(obj);
				obj->Start();

				SetTimer(mTimerDelayExit, 5000);
			}
			long mTimerDelayExit=0;
			void OnTimer(long id)
			{
				if (id == mTimerDelayExit)
				{
					PostQuitMessage();
					return;
				}

				__super::OnTimer(id);
			}

			LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp)
			{
				if (msg == BM_TEST)
				{
					//LogV(TAG,"index=%d", (int)wp);
				}

				return __super::OnMessage(msg, wp, lp);
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TestCreate)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<Handler>();
				AddChild(obj);

				PostQuitMessage();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	//测试AddChild验证其looper safe
	//多个looper同时对同一个handler进行AddChild
	//正常情况不应该这样使用
	TEST_METHOD(TestAddChild)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				class WorkLooper :public Looper
				{
				public:
					shared_ptr<Handler> mTestChild;
				protected:
					void OnCreate()
					{
						__super::OnCreate();

						AddChild(mTestChild);
						mTestChild = nullptr;
					}
				};

				auto obj = make_shared<Handler>();
				vector < shared_ptr<Looper>> loopers;
				for (int i = 0; i < 100; i++)
				{
					auto looper=make_shared<WorkLooper>();
					looper->mTestChild = obj;
					AddChild(looper);
					loopers.push_back(looper);
				}

				for(auto iter=loopers.begin();iter!=loopers.end();++iter)
				{
					(*iter)->Start();
				}


				PostQuitMessage();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	//普通场景
	TEST_METHOD(NormalCase)
	{
		{
			//LogV(TAG,"sizeof(CriticalSection)=%d", sizeof(CriticalSection));
			//正常情况下，只能用make_shared创建Handler及其子类
			//尽管不能发挥完全的功能，但要确保能在stack构造和析构
			Handler obj;
			Looper looper;
			auto h = make_shared<Handler>();
			auto l = make_shared<Looper>();
		}
	}

	//looper非常规用法
	TEST_METHOD(Looper_SpecialUsage)
	{
		{
			class MainLooper :public MainLooper_
			{
			protected:
				void OnCreate()
				{
					__super::OnCreate();

					Destroy();
				}
			};

			class WorkLooper :public Looper
			{
			public:
				WorkLooper()
				{
					SetObjectName("WorkLooper");
				}
			protected:
				void OnCreate()
				{
					__super::OnCreate();
					{
						long id = 0;
						SetTimer(id, 1);
					}

					Destroy();
				}
			};

			{
				//auto obj = make_shared<MainLooper>();
			}

			{
				auto event = make_shared<Event>(true, false);
				{
					auto obj = make_shared<MainLooper>();
					obj->SetExitEvent(event);

					obj->Create();
					{

						class WorkHandler :public Handler
						{
						public:
							WorkHandler()
							{
								SetObjectName("WorkHandler");
							}
						protected:
							void OnCreate()
							{
								__super::OnCreate();

								auto looper = make_shared<WorkLooper>();
								looper->Create(shared_from_this());
							}
						};

						make_shared<WorkHandler>()->Create(obj);
					}
					obj->Destroy();//由于looper Destroy()只是PostQuitMessage(0),所以要采用event确保MainLooper完全退出后才能进行后续的测试
				}
				event->Wait();
			}

			{
				//make_shared<MainLooper>()->StartRun();
			}

		}
	}

	TEST_METHOD(TestWin32Window)
	{
		for (int i = 0; i < 10; i++)
		{
			auto ret = SendMessage((HWND)(LONGLONG)i, WM_NULL, 0, 0);

			LogV(TAG,"[%04d].ret=%d.error=%d",i, ret,GetLastError());
		}
	}

	TEST_METHOD(TestSizeof)
	{
		string arr[] = {"1","12","123","1234",};
		LogV(TAG,"sizeof arr=%d,sizeof string=%d", sizeof(arr),sizeof(string));
		LogV(TAG,"sizeof(shared_ptr)=%d", sizeof(shared_ptr<Handler>));
		LogV(TAG,"sizeof(Handler)=%d", sizeof(Handler));
	}

	TEST_METHOD(CreateCrossLooperHandler)
	{
		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				auto looper = make_shared<Looper>();
				looper->Start();
				AddChild(looper);

				class Worker :public Handler
				{
				public:
					Worker()
					{
						SetObjectName("Worker");
						DG("Worker,this=%p", this);
					}
					~Worker()
					{
						DG("~Worker,this=%p", this);
					}
				protected:
					void OnCreate()
					{
						__super::OnCreate();

						DG("%s,threadId=%d", __func__,ShellTool::GetCurrentThreadId());

						SetTimer(mTimerTest, 200);
					}
					
					int mIndex = -1;

					void OnTimer(long id)
					{
						if (id == mTimerTest)
						{
							DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());

							++mIndex;
							if (mIndex == 5)
							{
								Looper::GetMainLooper()->PostQuitMessage();
							}
						}

						__super::OnTimer(id);
					}

					long mTimerTest = 0;
				};

				DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
				auto obj = make_shared<Worker>();
				looper->AddChild(obj);
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TestPointer)
	{
		class Handler2 :public Handler
		{
		public:
			int mX = 0;
		};

		auto obj = make_shared<Handler2>();
		auto obj2 = dynamic_pointer_cast<Handler2>(obj.get()->shared_from_this());
		if (obj == obj2)
		{
			LogV(TAG,"ok");
		}
		else
		{
			DW("fail");
		}
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

	TEST_METHOD(SendMessageSpeed)
	{
		static bool bindCPU = false;
		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();
				if (bindCPU)
				{
					HANDLE hThread = OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, GetCurrentThreadId());
					DWORD_PTR value = SetThreadAffinityMask(hThread, 0x0001);
				}
			}
		};

		//auto tlsLooper = Looper::BindTLSLooper();

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

			auto nc = 1000;// *1000;
			auto tick = ShellTool::GetTickCount64();
			for (int i = 0; i < nc; ++i)
			{
				obj->sendMessage(BM_NULL);
			}
			tick = ShellTool::GetTickCount64() - tick;
			LogV(TAG,"tick=%I64d", tick);
			obj->PostQuitMessage();
		}
		//event->Wait();
	}

	//由于Handler内部大量用到了shared_from_this(),所以不应该在stack上创建Handler对象,请使用make_shared来创建
	TEST_METHOD(CreateStackHandler)
	{

		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				Handler obj;
				//obj.Create();

				PostQuitMessage(1);
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
	}
	
	TEST_METHOD(weakMemory)
	{
		/*
理想情况下BigObject占用的内存在最后一个shared_ptr失效时释放
控制块在weak_ptr失效时释放

现在用make_shared时控制块和BigObject是一起分配的，
当shared_ptr失效时，只调用了BigObject的析构
当weak_ptr失效时才释放了控制块和BigObject占用的内存
		*/
		class BigObject
		{
		public:
			BigObject()
			{
				LogV(mTag, "%s(%p)", __func__, this);
				memset(mBuffer, 0, sizeof(mBuffer));
			}
			virtual ~BigObject()
			{
				LogV(mTag, "%s(%p)", __func__, this);
			}
		protected:
			BYTE mBuffer[1024 * 1024 * 100];
			string mTag = "BigObject";
		};
		/*
		https://zhuanlan.zhihu.com/p/579105226
只要std::weak_ptrs引用一个控制块(即，弱计数大于零)，该控制块必须继续存在。
只要控制块存在，包含它的内存就保持被分配的状态。
因此，由std::shared_ptr make函数分配的内存，在引用它的最后一个std::shared_ptr和最后一个std::weak_ptr被销毁之前不能被释放的。

如果对象类型非常大，并且销毁最后一个std::shared_ptr和销毁最后一个std::weak_ptr之间的时间间隔很长，
那么在销毁对象和释放它所占用的内存之间可能会出现延迟
		*/

		{
			weak_ptr<BigObject> wObj;
			{
				auto obj = make_shared<BigObject>();
				wObj = obj;
				int x = 0;
			}
			LogV(TAG, "shared_ptr end");
			wObj.reset();

		}
		int y = 0;
	}

	TEST_METHOD(handlerMemory)
	{
		class BigBuffer : public enable_shared_from_this<BigBuffer>
			, public sigslot::has_slots<>

		{
		public:
			BigBuffer()
			{
				mBuf = new BYTE[1024 * 1024 * 100];
			}
			~BigBuffer()
			{
				delete[]mBuf;
				mBuf = nullptr;
			}

		protected:
			LPBYTE mBuf = nullptr;
		};
		class HttpPost3 :public Handler
		{
			SUPER(Handler);
		public:
			HttpPost3()
			{
				LogV(mTag, "%s(%p)", __func__, this);
				mBigBuffer = make_shared<BigBuffer>();
				memset(mBuffer, 0, sizeof(mBuffer));
			}
			~HttpPost3()
			{
				LogV(mTag, "%s(%p)", __func__, this);
			}
		protected:
			BYTE mBuffer[1024 * 1024 * 10];
			string mTag = "HttpPost3";
			shared_ptr<BigBuffer> mBigBuffer;
		};

		class MainLooper :public MainLooper_
		{
		protected:
			long mTimer_test = 0;
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimer_test, 5000);//

				//DelayExit(10*1000);
				//PostQuitMessage(1);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_test)
				{
					auto obj = make_shared<HttpPost3>();
					AddChild(obj);
					obj->Destroy();

					KillTimer(mTimer_test);

					return;
				}
				__super::OnTimer(id);
			}
		};

		{
			auto obj = make_shared<MainLooper>();
			auto ret = obj->StartRun();
		}
		int x = 0;
	}


	TEST_METHOD(TestChannel)
	{
		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				class TcpChannel:public TcpClient
				{
					
				};

				auto obj = make_shared<TcpChannel>();
				AddChild(obj);
				Bundle info;
				info.Set("address", "123.58.180.8");
				info.Set("port", 80);
				obj->Connect(info);

				PostQuitMessage(0);
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
	}

	TEST_METHOD(fileLogger)
	{
		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				{
					LogD(TAG, u8"早期数据");

					auto obj = make_shared<FileLogger>();
					obj->setFilePath("d:/test/file.log",3*1024,false);
					
					obj->addDisableTags("main,onvif,");
					LogD("main", "main log");
					LogD("onvif", "onvif log");
					LogD("http", "http log");

					//obj->disableLevel(DT_DEBUG);
					LogD("http", "http log2");
					LogI("http", "http info");

					//obj->disableLevel(DT_DEBUG);

					//obj->disableLog();
					AddChild(obj);
					obj->Start();
					obj->sendMessage(BM_NULL);

					

					for (int i = 0; i < 4000;i++)
					{
						LogD(TAG, "test file logger[%04d]",i);
					}

					LogI(TAG, u8"mainLooper is gone...");

					DelayExit(1);
				}
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(TestLooper)
	{
		class MainLooper :public Looper
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				PostQuitMessage(1);
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 1);
	}

	TEST_METHOD(TestLooperQuit)
	{
		class MainLooper :public Looper
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				PostQuitMessage(1);

				class TestHandler :public Handler
				{
				public:
					TestHandler()
					{
						LogV(TAG,"%s", __func__);
					}
					~TestHandler()
					{
						LogV(TAG,"%s", __func__);
					}

				protected:
					void OnCreate()
					{
						__super::OnCreate();
						LogV(TAG,"%s", __func__);
					}
					void OnDestroy()
					{
						__super::OnDestroy();
						LogV(TAG,"%s", __func__);
					}
				};

				AddChild(make_shared<TestHandler>());
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 1);
	}
	
	TEST_METHOD(TestSendMessage)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				class WorkLooper :public Looper
				{
					void OnCreate()
					{
						__super::OnCreate();
					}

				public:
					//UINT mTestMessage = BindMessage();
				};

				PostQuitMessage();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	//确保能支持大量的Looper
	TEST_METHOD(TestManyLooper)
	{
		class WorkLooper :public Looper
		{
			void OnCreate()
			{
				__super::OnCreate();

				++gWorkLooperTimes;
				

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
				SetObjectName("TestLooper");
				mThreadName = GetObjectName();
			}
		protected:
			void OnCreate()
			{
				__super::OnCreate();
				LogV(TAG,"%s", __func__);

				auto obj = _MObject(TestLooper, "TestLooper");
				if (obj)
				{
					LogV(TAG,"obj is ok");
				}
				else
				{
					DW("obj is null");
				}
				int x = 0;
			}
			
			void OnDestroy()
			{
				LogV(TAG,"%s", __func__);
				__super::OnDestroy();
			}
		};

		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				LogV(TAG,"%s", __func__);
				__super::OnCreate();

				{
					auto looper = make_shared<TestLooper>();
					AddChild(looper);
					looper->Start();
				}

				PostQuitMessage(1);
			}
			void OnDestroy()
			{
				LogV(TAG,"%s", __func__);
				__super::OnDestroy();
			}
		};

		auto obj = make_shared<MainLooper>();
		auto ret = obj->StartRun();
		Assert::AreEqual(ret, 1);
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
			item = obj;
		}

		tick = ShellTool::GetTickCount64() - tick;
		LogV(TAG,"tick=%I64d", tick);
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

		LogV(TAG,"sizeof(string) = %d", sizeof(string));//40
		std::map<long*, weak_ptr<Handler>> mChildren;
		LogV(TAG,"sizeof(map) = %d", sizeof(mChildren));//24
		LogV(TAG,"sizeof(Handler) = %d", sizeof(Handler));
		LogV(TAG,"sizeof(Handler2) = %d", sizeof(Handler2));

		LogV(TAG,"sizeof(shared_ptr)=%d", sizeof(shared_ptr<std::map<UINT, shared_ptr<tagTimerNode>>>));

		//shared_ptr<Handler> obj;
		//LogV(TAG,"shared_ptr size=%d", sizeof(obj));

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
				LogV(TAG,"%s,mTimerInConstructor", __func__);
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
						long id = 0;
						LogV(TAG,"timerId#%d=%d", i + 1, SetTimer(id,2000));
					}
					//return;

					SetTimer(mTimerTest, 1000);
					SetTimer(mTimerFeed, 2000);
				}

				void OnTimer(long id)
				{
					//LogV(TAG,"%s,id=%d", __func__, id);

					if (id == mTimerTest)
					{
						static int idx = -1;
						++idx;
						LogV(TAG,"id=%d,test idx=%04d", id, idx);
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
						LogV(TAG,"id=%d,feedDog,idx=%04d", id, idx);
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

				long timerId = 0;
				auto ret = SetTimer(timerId,1);
				timerId = -2;
				KillTimer(timerId);
				//LogV(TAG,"%d", );

				AddChild(make_shared<WatchDog>());
			}
		};

		auto looper = make_shared<MainLooper>();
		auto ret = looper->StartRun();
		LogV(TAG,"exit code=%d", looper->GetQuitCode());
	}

	//不需要返回时，也没有入参
	TEST_METHOD(TestMain_Simple)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();
				PostQuitMessage(2018);
			}

		};

		//int main()
		//{
			auto ret=make_shared<MainLooper>()->StartRun();
			Assert::IsTrue(ret == 2018);
			//return 0;
		//}
	}

	//需要返回值，有入参
	TEST_METHOD(TestMain)
	{
		class MainLooper :public MainLooper_
		{
		public:
			void ParseCommandLine(int argc, const char *argv[])
			{
				for (int i = 0; i < argc; i++)
				{
					auto item = argv[i];
					if (i == 0)
					{
						mApp = item;
					}
					else
					{
						mArgs.emplace_back(item);
					}
				}
			}
		protected:
			string mApp;
			vector<string> mArgs;

			void OnCreate()
			{
				__super::OnCreate();
				PostQuitMessage(2018);
			}
		};

		//模拟测试int main(int argc,char*argv[]);
		const char *argv[] = { "app","arg0","arg1", };
		int argc = COUNT_OF(argv);
		//int main(int argc,char*argv[])
		//{
				auto obj = make_shared<MainLooper>();
				obj->ParseCommandLine(argc, argv);

				obj->StartRun();
				auto ret = obj->GetQuitCode();
				LogV(TAG,"quit code=%d", ret);
				ASSERT(ret == 2018);
				//return ret;
		//}
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
				LogV(TAG,"%s", __func__);
			}
			~DelayLooper()
			{
				LogV(TAG,"%s", __func__);
			}

			class DelayHandler :public Handler
			{
			public:
				DelayHandler()
				{
					LogV(TAG,"%s", __func__);
				}
				~DelayHandler()
				{
					LogV(TAG,"%s", __func__);
				}
			protected:
				void OnDestroy()
				{
					LogV(TAG,"%s", __func__);
					__super::OnDestroy();

				}

				void OnCreate()
				{
					__super::OnCreate();

					mDelayRef = shared_from_this();
					SetTimer(mTimerDelay,3000);
				}

				void OnTimer(long id)
				{
					if (id == mTimerDelay)
					{
						LogV(TAG,"%s", __func__);
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

				LogV(TAG,"%s", __func__);
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
						//LogV(TAG,"this=%p,value=%d",this, mValue);
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
				LogV(TAG,"%s", __func__);
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
				//LogV(TAG,"handler=%p", handler);
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
						LogV(TAG,"mHandler=%p", mHandler.get());
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
				LogV(TAG,"%s", __func__);
			}
			~WorkHandler()
			{
				LogV(TAG,"%s", __func__);
			}

			void OnCreate()
			{
				__super::OnCreate();
				SetTimer(mTimerTest,100);
			}

			void OnDestroy()
			{
				LogV(TAG,"%s", __func__);
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
					LogV(TAG,"BM_TEST,idx=%04d", idx);
					return 0;
				}
				}

				return __super::OnMessage(msg, wp, lp);
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					LogV(TAG,"%s", __func__);
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

				LogV(TAG,"WorkLooper PostQuitMessage");
				looper->PostQuitMessage();

				SetTimer(mTimerDelay,3000);
				SetTimer(mTimerTest,500);
				DT("mTimerDelay=%d,mTimerTest=%d", mTimerDelay, mTimerTest);
			}

			void OnTimer(long id)
			{
				LogV(TAG,"%s,id=%d", __func__, id);
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

					LogV(TAG,"%s,PostQuitMessage", mThreadName.c_str());
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

	TEST_METHOD(StringTool_)
	{
		{
			class Tool {
			public:
				static string Format(const char* fmt, ...)
				{
					if (!fmt || fmt[0] == 0)
					{
						return "";
					}

					string result;
					va_list list;
					va_start(list, fmt);

					int ret = 0;
					ret = vsnprintf(nullptr, 0, fmt, list);
					printf("fmt#1=[%s],ret=%d\r\n", fmt, ret);

					va_end(list);
					va_start(list, fmt);

					ret = vsnprintf(nullptr, 0, fmt, list);
					printf("fmt#2=[%s],ret=%d\r\n", fmt, ret);

					if (ret > 0)
					{
						result.resize(ret + 1);
						ret = vsnprintf((char*)result.data(), ret + 1, fmt, list);
						//printf("vsnprintf ret=%d\r\n", ret);

						auto len = ret;// MIN(ret, result.size());
						//printf("len=%d\r\n", (int)len);
						result.erase(len);
					}

					va_end(list);

					return result;
				}
			};

			int idx = 123456789;
			auto text = Tool::Format("hello%d", idx);
			printf("text=[%s]\r\n", text.c_str());

		}

		{
			int idx = -1;

			LogV(TAG, "%s,sizeof(int)=%d", __func__, (int)sizeof(int));
			++idx;

			//LogV(TAG, "fmt#1");
			auto text = StringTool::Format("hello%d", idx);
			int x = 0;

		}

		{
			string sz = "##";
			StringTool::Replace(sz, "#", "##");
			ASSERT(sz == "####");
		}

		{
			string sz = ".";
			StringTool::Replace(sz, ".", "#");
			ASSERT(sz == "#");
		}

		{
			string sz = "192.168.1.3";
			StringTool::Replace(sz, ".", ",");
			ASSERT(sz == "192,168,1,3");
		}
	}

	TEST_METHOD(TestSTL)
	{
		list<int> items;
		items.push_back(1);
		items.push_back(2);
		items.push_back(3);
		items.push_back(4);
		
		for (auto iter = items.rbegin(); iter != items.rend(); ++iter)
		{
			LogV(TAG,"%d", *iter);
		}
	}

	TEST_METHOD(TestStringFormat)
	{
		auto sz = StringFormat("hello,%d,%s", 123,"bear");
		LogV(TAG,"%s", sz.c_str());
	}

	TEST_METHOD(TestCpp)
	{
		class Base
		{
		public:
			Base()
			{

			}
			Base(int x):mX(x)
			{

			}
			int mX = 0;
		};

		class Derive :public Base
		{
			SUPER(Base);

		};

		{
			auto d = make_shared<Derive>(123);
		}

		auto obj = make_shared<Base>();
		auto obj2 = make_shared<Base>();
		auto sp = obj;
		auto sp2 = obj2;
		sp = sp2;

		if (sp == sp2)
		{
			LogV(TAG,"is equal");
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

	TEST_METHOD(string_replace)
	{
		{
			string sz = "ABCDABCD";
			StringTool::Replace(sz, "AB", "");
			Assert::IsTrue(sz == "CDCD");
		}

		{
			string sz = "ABCDABCD";
			StringTool::Replace(sz, "AB", "ABC");
			Assert::IsTrue(sz == "ABCCDABCCD");
		}
		{
			string sz = "AAA";
			StringTool::Replace(sz, "A", "");
			Assert::IsTrue(sz.empty());
		}
		{
			string sz = "AAA";
			StringTool::Replace(sz, "AA", "");
			Assert::IsTrue(sz=="A");
		}
		{
			string sz = "AAA";
			StringTool::Replace(sz, "AA", "A");
			Assert::IsTrue(sz == "AA");
		}

	}

	TEST_METHOD(Bundle_Test)
	{
		int ret = -1;
		size_t xx = ret;
		LogV(TAG,"xx=%u", xx);

		Bundle obj;
		LogV(TAG,"pack#1=[%s]", obj.Pack().c_str());
		obj.Set("name", "bear");

		Bundle obj2;
		obj2 = obj;
		LogV(TAG,"name=%s", obj2.GetString("name").c_str()); ;

		LogV(TAG,"pack=[%s]", obj2.Pack().c_str());
	}

	//显示CoreLooper典型用法
	TEST_METHOD(DemoUsage)
	{
		class WorkHandler :public Handler
		{
		public:
			WorkHandler()
			{
				LogV(TAG,"%s,this=%p", __func__, this);
			}

			~WorkHandler()
			{
				LogV(TAG,"%s,this=%p", __func__, this);
			}

			void SetTestTimer()
			{
				SetTimer(mTimerTest,300);
			}

		protected:
			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					static int idx = -1;
					++idx;
					LogV(TAG,"%s,idx=%d", __func__, idx);

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

				SetTimer(mTimerTest,1000);
				SetTimer(mTimerDelayQuit,5 * 1000);
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					KillTimer(mTimerTest);

					auto obj = mHandler.lock();
					if (obj)
					{
						LogV(TAG,"destroy %p", obj.get());
						obj->Destroy();

						//测试handler destroy之后timer是否能正常工作
						obj->SetTestTimer();

						mHandlerRef = obj;
						SetTimer(mTimerDelayRelease,1500);
					}
				}
				else if (id == mTimerDelayRelease)
				{
					KillTimer(mTimerDelayRelease);

					LogV(TAG,"release mHandlerRef");
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

TEST_CLASS(AsyncTask_UnitTest)
{
public:
	TEST_METHOD(AsyncTask_Demo)
	{
		class DemoTask :public AsyncTask
		{
		public:
			DemoTask()
			{
				LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
			}
			~DemoTask()
			{
				DG("%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
			}
		protected:
			virtual void OnPreExecute()
			{
				LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
			}

			virtual void OnPostExecute()
			{
				LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());

				Looper::GetMainLooper()->PostQuitMessage();
			}

			virtual void Run()
			{
				LogV(TAG,"%s,threadId=%d", __func__, ShellTool::GetCurrentThreadId());
			}

		};

		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				make_shared<DemoTask>()->Execute();
				make_shared<DemoTask>()->Execute();
			}
		};

		make_shared<MainLooper>()->StartRun();

	}

};

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

					LogV(TAG,"postDelayedRunnable");
					postDelayedRunnable(make_shared<DelayExitRunnable>(), 1000);
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
				SetMainLooper(this);
				mThreadName = "MainLooper";
			}

		protected:
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimerDelayExit, 2100);
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
				LogV(TAG,"%s", __func__);
				static int idx = -1;
				++idx;

				if (mBuddy)
				{
					Looper::CurrentLooper()->cancelRunnable(mBuddy);
				}
			}

			shared_ptr<WifiHelper> mBuddy;
		};

		//auto tlsLooper = Looper::BindTLSLooper();
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
		static long gTestTimes = 1000;// *1000;
		static long mTimes = 0;
		class WifiHelper :public Runnable
		{
		public:
			void Run()
			{
				if (++mTimes == gTestTimes)
				{
					//DW("post quit message");
					Looper::CurrentLooper()->PostQuitMessage();
				}

				//LogV(TAG,"mTimes=%d", mTimes);
			}

		};

		LONGLONG tick = 0;
		//auto tlsLooper = Looper::BindTLSLooper();
		auto event = make_shared<Event>();
		{
			auto looper = make_shared<MainLooper_>();
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
		LogV(TAG,"tick = %I64d", tick);//release版100万次约1.2秒,debug版约9.2秒
		if (tick > 0)
		{
			long speed = (long)(gTestTimes *1000.0 / tick);
			LogV(TAG,"speed = %d/s", (int)speed);//release版88万/秒,debug版10万/秒
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
				LogV(TAG,"%s,idx=%04d", __func__, idx);
			}

			bool IsExecuted()const
			{
				return mExecuted;
			}
		protected:
			bool mExecuted = false;
		};

		//auto tlsLooper = Looper::BindTLSLooper();
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

		LogV(TAG,"obj->IsExecuted()=%d", obj->IsExecuted());
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
					LogV(TAG,"%s,idx=%04d", __func__, idx);
				}

				bool IsExecuted()const
				{
					return mExecuted;
				}
			protected:
				bool mExecuted = false;
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
				SetObjectName("MainLooper");
				mThreadName = GetObjectName();

				SetMainLooper(this);
			}

		protected:
			void OnCreate()
			{
				__super::OnCreate();

				class WorkLooper :public Looper
				{
				public:
					WorkLooper()
					{
						LogV(TAG,"%s,this=%p", __func__, this);
					}
					~WorkLooper()
					{
						LogV(TAG,"%s,this=%p", __func__, this);
					}
				};
				class Worker :public Runnable
				{
				public:
					Worker()
					{
						LogV(TAG,"%s,this=%p", __func__, this);
					}
					virtual ~Worker()
					{
						LogV(TAG,"%s,this=%p", __func__, this);
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
	//必须在触发OnCreate之后才能调用
	TEST_METHOD(Timer_SetTimerPerformance)
	{
		class MainLooper :public MainLooper_
		{
		protected:
			void OnCreate()
			{
				__super::OnCreate();

				class WifiManager :public Handler
				{
					void OnCreate()
					{
						__super::OnCreate();

						auto tick = ShellTool::GetTickCount64();
						for (int i = 0; i < 
							//1000000
							1000
							; ++i)
						{
							long id = 0;
							SetTimer(id,10 * 1000);
						}
						tick= ShellTool::GetTickCount64() - tick;
						LogV(TAG,"tick=%I64d", tick);

						CurrentLooper()->PostQuitMessage();
					}
				};

				AddChild(make_shared<WifiManager>());
				//CurrentLooper()->PostQuitMessage();
			}
		};

		{
			auto looper = make_shared<MainLooper>();
			looper->StartRun();
		}
	}

	TEST_METHOD(ParseHttpRequest)
	{
		const char *ack =
			"HTTP/1.1 200 OK\r\n"
			"Date: Thu, 09 Aug 2018 07:49:02 GMT\r\n"
			"Server: Apache\r\n"
			"Last-Modified: Mon, 12 Jun 2017 03:35:34 GMT\r\n"
			"ETag: \"2921db3-2ad-551bb0303d1b7\"\r\n"
			"Accept-Ranges: bytes\r\n"
			"Vary: Accept-Encoding,User-Agent\r\n"
			"Content-Encoding: gzip\r\n"
			"Content-Length: 380\r\n"
			"Keep-Alive: timeout=15, max=300\r\n"
			"Connection: Keep-Alive\r\n"
			"Content-Type: application/javascript\r\n"
			"\r\n"
			;
	}
	TEST_METHOD(TestBase)
	{
		class Base
		{
		public:
			Base()
			{

			}

			Base(const char *name)
			{

			}
		};

		class SubA :public Base
		{
			using Base::Base;//vs2013不支持这种用法,vs2017和hi3516支持

		};

		auto objA = make_shared<SubA>("hello");
	}

	TEST_METHOD(TestFloat)
	{
		auto v = 0.0000001;
		double sum = 0;
		for (int i = 0; i < 10000; i++)
		{
			sum += v;
		}

		LogV(TAG,"sum=%.4f", sum);
	}

	TEST_METHOD(TestWait)
	{
		DWORD bytes = 0;
		ULONG_PTR ptr = NULL;
		LPOVERLAPPED ov = NULL;
		HANDLE handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		DWORD ms = 1;

		for (int i = 0; i < 100; i++)
		{
			auto tick = ShellTool::GetTickCount64();
			auto ret = GetQueuedCompletionStatus(handle, &bytes, &ptr, &ov, ms);
			tick = ShellTool::GetTickCount64() - tick;
			LogV(TAG,"tick[%04d]=%lld", i,tick);//经测试,tick大部分为0,有时为15或16,是windows线程切换引起的
		}

		CloseHandle(handle);
	}

	//测试发现windows下timer精度在16ms左右，主要是线程切换引起
	TEST_METHOD(TestDelta)
	{
		class MainLooper :public MainLooper_
		{
			SUPER(MainLooper_);

			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimerTest, 5);
				mStartTick = ShellTool::GetTickCount64();
			}

			void OnTimer(long id)
			{
				if (id == mTimerTest)
				{
					auto tickNow = ShellTool::GetTickCount64();
					if (mDebugTick)
					{
						auto delay = tickNow - mDebugTick;
						if (delay > 10)
						{
							static int idx = -1;
							++idx;
							LogW(TAG,"phone timer delay[%04d]=%lld", idx, delay);
						}

						mDebugTick = tickNow;
					}
					else
					{
						mDebugTick = tickNow;
					}

					if (tickNow > mStartTick + 10 * 1000)
					{
						PostQuitMessage();
					}

					return;
				}

				__super::OnTimer(id);
			}

			long		mTimerTest = 0;
			ULONGLONG	mDebugTick = 0;
			ULONGLONG	mStartTick=0;
		};

		make_shared<MainLooper>()->StartRun();
	}
	
};

TEST_CLASS(StringTool_UnitTest)
{
	TEST_METHOD(Format)
	{

		{
			string value = "123";
			int dots = 0;
			auto pos = value.find(".");/* 小数点后位数 */
			if (pos != string::npos)
			{
				dots = value.length() - pos-1;
			}
			int x = 0;
		}
		auto text = StringTool::Format(
			"hello,this is a very long text,value=%d",1000
		);
		
		LogV(TAG,"text#1=%s", text.c_str());

		StringTool::AppendFormat(text, ",name=%s", "xwp");
		LogV(TAG,"text#2=%s", text.c_str());

	}
};

TEST_CLASS(Log)
{
	TEST_METHOD(snprintf_)
	{
		int nc = 1000 * 1000;
		char buf[200];
		buf[0] = 1;
		buf[1] = 0;

		auto tick = ShellTool::GetTickCount64();
		for (int i = 0; i < nc; ++i)
		{
			_snprintf(buf, sizeof(buf), "hello,%d,this is a very long log for test,d=%d", i,i);
		}

		tick = ShellTool::GetTickCount64() - tick;
		LogV("Log", "snprintf tick=%lld,nc=%d", tick, nc);//snprintf tick=1735,nc=1000000
	}

	TEST_METHOD(WM_COPYDATA_)
	{
		auto tick = ShellTool::GetTickCount64();

		static const auto* gTitle = _T("DT2020 ");
		int nc = 1000 * 100;
		BYTE buf[2];
		buf[0]=1;
		buf[1] = 0;

		for (int i = 0; i < nc; ++i)
		{
			static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
			if (!IsWindow(hwnd))
			{
				hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
			}

			COPYDATASTRUCT cs;
			cs.dwData = 0;
			cs.cbData = sizeof(buf);
			cs.lpData = buf;

			DWORD ret = 0;
			::SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cs, SMTO_BLOCK, 10 * 1000, (PDWORD_PTR)&ret);
		}

		tick = ShellTool::GetTickCount64() - tick;
		LogV("Log", "WM_COPYDATA tick=%lld,nc=%d", tick, nc);//WM_COPYDATA tick=2250,nc=100000,注意只测试了10万次，比mutex慢了约20倍
	}

	TEST_METHOD(CriticalSection)
	{
		CRITICAL_SECTION obj = {0};
		InitializeCriticalSection(&obj);

		auto tick = ShellTool::GetTickCount64();

		int nc = 1000 * 1000;
		for (int i = 0; i < nc; ++i)
		{
			EnterCriticalSection(&obj);
			LeaveCriticalSection(&obj);
		}

		tick = ShellTool::GetTickCount64() - tick;
		LogV("Log", "CS tick=%lld,nc=%d", tick, nc);//CS tick=16,nc=1000000,可以看到cs比mutex快很多
	}

	TEST_METHOD(Mutex)
	{

		auto mutex = CreateMutex(
			NULL,              // default security attributes
			FALSE,             // initially not owned
			NULL);             // unnamed mutex
		
		auto tick = ShellTool::GetTickCount64();

		int nc = 1000 * 1000;
		for (int i = 0; i < nc; ++i)
		{
			auto ret = WaitForSingleObject(mutex, INFINITE);
			if (ret == WAIT_OBJECT_0)
			{
				ReleaseMutex(mutex);
			}
		}

		tick = ShellTool::GetTickCount64()-tick;
		LogV("Log", "Mutex,tick=%lld,nc=%d", tick, nc);//Mutex,tick=1359,nc=1000000

	}

	TEST_METHOD(format)
	{
		class Test
		{
		public:
			static int fn(const char* lpszFormat, ...)
			{
				char szMsg[5] = {0};

				va_list argList;
				va_start(argList, lpszFormat);
				try {
					vsnprintf(szMsg, sizeof(szMsg) - 1, (char*)lpszFormat, argList);
				}
				catch (...)
				{
					szMsg[0] = 0;
				}
				va_end(argList);

				return 0;
			}

		};

		Test::fn("he,%d", 12345);

	}

	TEST_METHOD(TestDT)
	{
		LogV(TAG,"DV");
		DT("DT");
		DG("DG");
		DW("DW");
		DE("DE");
	}
	
	TEST_METHOD(SharedMemory_Writer)
	{
		ULONGLONG bytes = 1024 * 4*1024;
		auto fd=CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, (DWORD)bytes, _T("Local\\CoreLooper"));
		//auto fd2 = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bytes, _T("Local\\CoreLooper"));
		//LogV(TAG,"fd=%p,fd2=%p", fd,fd2);

		auto d = (LPBYTE)MapViewOfFile(fd,FILE_MAP_ALL_ACCESS,0,0, (SIZE_T)bytes);
		d[0] = 0x12;
		d[1] = 0x34;
		//d[bytes] = 0xCD;//test overflow,vs can detect it
		ULONGLONG times = 1000;
		auto tick = ShellTool::GetTickCount64();
		for (ULONGLONG i = 0; i < times; i++)
		{
			for (int i = 0; i < bytes; i++)
			{
				d[i] = i%256;
			}
		}

		tick = ShellTool::GetTickCount64()-tick;
		ULONGLONG totalBytes = times * bytes;
		double speed = 0;
		if (tick > 0)
		{
			speed = (double)(totalBytes * 1000 / tick/1024/1024);
		}
		LogV("Log","tick=%lld,totalBytes =%lld(%.1f MB/S)", tick, totalBytes,speed);
		int x = 0;

	}
};


TEST_CLASS(STL)
{

	TEST_METHOD(MultiMap)
	{
#define strcasecmp _stricmp

		struct StrCaseCompare
		{
			bool operator()(const string& __x, const string& __y) const
			{
				return strcasecmp(__x.data(), __y.data()) < 0;
			};
		};

		multimap<string, string, StrCaseCompare> items;
		items.emplace("player","tom");
		items.emplace("player", "jerry");

		for (auto& item : items)
		{
			LogV(TAG,"%s=%s", item.first.c_str(), item.second.c_str());
		}

		auto iter = items.find("Player");
		if (iter != items.end())
		{
			LogV(TAG,"value=%s", iter->second.c_str());
		}

	}
	
	TEST_METHOD(StdMove)
	{
		string text = "hello";
		string sz = std::move(text);
		int x = 0;
	}

	TEST_METHOD(type_size)
	{

		LogV(TAG, "sizeof(uint8_t)=%zu", sizeof(uint8_t));
	}
};

TEST_CLASS(_Udp)
{
public:
	TEST_METHOD(udpServer)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				{
					auto obj = make_shared<UdpServer>();
					obj->StartServer(2021);
				}
				{
					auto client = make_shared<UdpClient_Windows>();
					//client->
				}
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(tan_)
	{
		const double PI = 3.14159265;
		int degree = 6;
		auto v=tan(PI*degree/180);
		LogV(TAG, "tan(%d)=%.3f", degree,v);

		auto r=40*tan(PI * degree / 180);
		LogV(TAG, "r=%.3f", r);

	}

};

}



