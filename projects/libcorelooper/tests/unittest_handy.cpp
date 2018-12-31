#include "stdafx.h"
#include "CppUnitTest.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <utility>
#include <algorithm>
#include <chrono>
#include <memory>
#include <cmath>
#include <random>
#include <iostream>
#include <memory>
#include <functional>

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
*/

namespace UnitTest
{

typedef std::pair<int64_t, int64_t> TimerId;
typedef std::function<void()> Task;
struct TimerRepeatable {
	int64_t at;  // current timer timeout timestamp
	int64_t interval;
	TimerId timerid;
	Task cb;
};

struct util {
	static int64_t timeMicro();
	static int64_t timeMilli() { return timeMicro() / 1000; }
};

int64_t util::timeMicro() {
	chrono::time_point<chrono::system_clock> p = chrono::system_clock::now();
	return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
}


class EventLooper
{
public:
	void loop();
	void loop_once(int64_t waitMs) {
		loop_onceEx(min(waitMs, nextTimeout_));
		handleTimeouts();
	}

	TimerId runAt(int64_t milli, Task &&task, int64_t interval);
	bool cancel(TimerId timerid);

	EventLooper &exit();
	TimerId runAt(int64_t milli, const Task &task, int64_t interval = 0) { return runAt(milli, Task(task), interval); }
	TimerId runAfter(int64_t milli, const Task &task, int64_t interval = 0) { return runAt(util::timeMilli() + milli, Task(task), interval); }
	TimerId runAfter(int64_t milli, Task &&task, int64_t interval = 0) { return runAt(util::timeMilli() + milli, std::move(task), interval); }
protected:
	void loop_onceEx(int64_t waitMs);

	void handleTimeouts();
	void refreshNearest(const TimerId *tid = NULL);
	void repeatableTimeout(TimerRepeatable *tr);

	std::map<TimerId, TimerRepeatable> timerReps_;
	std::map<TimerId, Task> timers_;
	std::atomic<int64_t> timerSeq_;
	int64_t nextTimeout_ = 0;
	std::atomic<bool> exit_ = false;

	HANDLE mHandle = CreateEvent(NULL, true, FALSE, _T(""));// INVALID_HANDLE_VALUE;
};

void EventLooper::handleTimeouts() {
	int64_t now = util::timeMilli();

#ifdef _M_X64
	int shiftBits = 62;
#else
	int shiftBits = 30;
#endif
	TimerId tid{ now, 1L << shiftBits };
	while (timers_.size() && timers_.begin()->first < tid) {
		Task task = move(timers_.begin()->second);
		timers_.erase(timers_.begin());
		task();
	}
	refreshNearest();
}

void EventLooper::refreshNearest(const TimerId *tid) {
	if (timers_.empty()) {
		nextTimeout_ = 1 << 30;
	}
	else {
		const TimerId &t = timers_.begin()->first;
		nextTimeout_ = t.first - util::timeMilli();
		nextTimeout_ = nextTimeout_ < 0 ? 0 : nextTimeout_;
	}
}

void EventLooper::repeatableTimeout(TimerRepeatable *tr) {
	tr->at += tr->interval;
	tr->timerid = { tr->at, ++timerSeq_ };
	timers_[tr->timerid] = [this, tr] { repeatableTimeout(tr); };
	refreshNearest(&tr->timerid);
	tr->cb();
}

TimerId EventLooper::runAt(int64_t milli, Task &&task, int64_t interval)
{
	if (exit_) {
		return TimerId();
	}
	if (interval) {
		TimerId tid{ -milli, ++timerSeq_ };
		TimerRepeatable &rtr = timerReps_[tid];
		rtr = { milli, interval, {milli, ++timerSeq_}, move(task) };
		TimerRepeatable *tr = &rtr;
		timers_[tr->timerid] = [this, tr] { repeatableTimeout(tr); };
		refreshNearest(&tr->timerid);
		return tid;
	}
	else {
		TimerId tid{ milli, ++timerSeq_ };
		timers_.insert({ tid, move(task) });
		refreshNearest(&tid);
		return tid;
	}
}

bool EventLooper::cancel(TimerId timerid) {
	if (timerid.first < 0) {
		auto p = timerReps_.find(timerid);
		auto ptimer = timers_.find(p->second.timerid);
		if (ptimer != timers_.end()) {
			timers_.erase(ptimer);
		}
		timerReps_.erase(p);
		return true;
	}
	else {
		auto p = timers_.find(timerid);
		if (p != timers_.end()) {
			timers_.erase(p);
			return true;
		}
		return false;
	}
}

EventLooper &EventLooper::exit()
{
	exit_ = true;
	SetEvent(mHandle);
	return *this;
}

void EventLooper::loop() {
	while (!exit_)
		loop_once(10000);
	timerReps_.clear();
	timers_.clear();
	loop_once(0);
}

void EventLooper::loop_onceEx(int64_t waitMs) {
	//int64_t ticks = util::timeMilli();
	DWORD ms = (DWORD)waitMs;
	WaitForSingleObject(mHandle, ms);
	//epoll_wait(fd_, activeEvs_, kMaxEvents, waitMs);
}

TEST_CLASS(Test_Handy)
{
	TEST_METHOD(TestHandy)
	{
		auto obj = make_shared<EventLooper>();

		auto tick = ShellTool::GetTickCount64();
		TimerId id = obj->runAt(util::timeMilli(),
			[]()
		{
			static int idx = -1;
			++idx;
			DV("timer,idx=%04d", idx);
		}
			, 1000
			);

		/*
		auto nc = 1000 * 1000;
		DV("begin settimer,times=%d", nc);
		for (int i = 0; i < nc; i++)
		{
			TimerId id = obj->runAt(util::timeMilli(),
				[]()
			{
				static int idx = -1;
				++idx;
				DV("timer,idx=%04d", idx);
			}
				, 1000
				);
		}

		tick = ShellTool::GetTickCount64() - tick;
		DV("end   settimer,tick=%lld", tick);//1百万只要450ms左右
		//*/

		obj->runAfter(10 * 1000, [&]() { obj->exit(); });

		obj->loop();

	}

	void abssort(double* x, unsigned n) {
		std::sort(x, x + n,
			[](double a, double b) {
			return (std::abs(a) < std::abs(b));
		}
		);
	}

	TEST_METHOD(TestLambda)
	{
		double arr[] = {0.1,12.5,3.4,9.4,4.5,};
		abssort(arr, COUNT_OF(arr));

		for(int i=0;i<COUNT_OF(arr);i++)
		{
			//DV("arr[%d]=%04f", i, arr[i]);
		}

		int age = 12;
		string name = "b";
		auto func = [&age,&name]
		{
			age = 1;
			name="bear";
		};

		func();
		DV("age=%d", age);
	}

	TEST_METHOD(TestBind)
	{
		class Test
		{
		public:
		static void f(int n1, int n2, int n3, const int& n4, int n5)
		{
			std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
		}

		static int g(int n1)
		{
			return n1;
		}

		struct Foo {
			void print_sum(int n1, int n2)
			{
				std::cout << n1 + n2 << '\n';
			}
			int data = 10;
		};

		int main()
		{
			using namespace std::placeholders;  // 对于 _1, _2, _3...

			// 演示参数重排序和按引用传递
			int n = 7;
			// （ _1 与 _2 来自 std::placeholders ，并表示将来会传递给 f1 的参数）
			auto f1 = std::bind(f, _2, _1, 42, std::cref(n), n);
			n = 10;
			f1(1, 2, 1001); // 1 为 _1 所绑定， 2 为 _2 所绑定，不使用 1001
							// 进行到 f(2, 1, 42, n, 7) 的调用

			// 嵌套 bind 子表达式共享占位符
			auto f2 = std::bind(f, _3, std::bind(g, _3), _3, 4, 5);
			f2(10, 11, 12); // 进行到 f(12, g(12), 12, 4, 5); 的调用

			// 常见使用情况：以分布绑定 RNG
			std::default_random_engine e;
			std::uniform_int_distribution<> d(0, 10);
			std::function<int()> rnd = std::bind(d, e); // e 的一个副本存储于 rnd
			for (int n = 0; n < 10; ++n)
				std::cout << rnd() << ' ';
			std::cout << '\n';

			// 绑定指向成员函数指针
			Foo foo;
			auto f3 = std::bind(&Foo::print_sum, &foo, 95, _1);
			f3(5);

			// 绑定指向数据成员指针
			auto f4 = std::bind(&Foo::data, _1);
			std::cout << f4(foo) << '\n';

			// 智能指针亦能用于调用被引用对象的成员
			std::cout << f4(std::make_shared<Foo>(foo)) << '\n'		<< f4(std::make_unique<Foo>(foo)) << '\n';
		}

		};
	}

	static void PrintA(int a)
	{
		DV("a=%d", a);
	}

	TEST_METHOD(TestFunction_Static)
	{
		std::function<void(int a)> func;
		func = PrintA;
		func(2);
	}

	struct Foo {
		Foo(int num) : num_(num) {}
		void Add(int i) const
		{
			DV("add = %d", num_ + i);
		}
		void Sub(int i) const
		{
			DV("sub = %d", num_ - i);
		}
		int num_;
	};

	TEST_METHOD(TestFunction_ClassApi)
	{
		std::function<void(const Foo&, int)> fn;
		fn = &Foo::Add;
		//fn = &Foo::Sub;
		//fn = nullptr;
		Foo foo(2);
		fn(foo, 1);
	}

	static void Test(int x, int y) {
		DV("x=%d,y=%d", x, y);
	}
	TEST_METHOD(TestBind2)
	{
		{
			auto fn = std::bind(&Test, 12, 34);
			fn();
		}

	}
};


}

