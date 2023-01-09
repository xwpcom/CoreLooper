#include "stdafx.h"
#include "CppUnitTest.h"
#include "loger2.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core2;


namespace Bear {
using namespace Core;
namespace Core2 {
static const char* TAG = u8"core2";

TEST_CLASS(Log_)
{
	template<class T>
	void swap(T& a, T& b)
	{
		//decltype(a) t = a;
		auto t = a;
		a = b;
		b = t;
	};

	TEST_METHOD(templateBase)
	{
		{
			int x = 1, y = 2;
			swap(x, y);
			logV(TAG) << "x=" << x << ",y=" << y;
		}
		{
			string x = "hello", y = "world";
			swap(x, y);
			logV(TAG) << "x=" << x << ",y=" << y;
		}
	}

	TEST_METHOD(loger)
	{
		int year = 2023;
		/*

		auto tick = ShellTool::GetTickCount64();
		for (int i = 0; i < 1000000;i++)
		{
			LogerV(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";
		}

		tick = ShellTool::GetTickCount64()-tick;
		LogV(TAG, "tick=%lld", tick);
		*/

		logV(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";
		logD(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";
		logI(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";
		logW(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";
		logE(TAG) << u8"hello" << " world " << year << u8" 新年快乐! ";

		string tag = u8"stringLog";
		logV(tag) << u8"hello" << " world " << year << u8" 新年快乐! ";
		//TraceL << u8"second line";
	}

};

void Display(int& i)
{
	logV(TAG) << __func__<<"#int&";
}

void Display(int&& i)
{
	logV(TAG) << __func__ << "#int&&";
}

template<class T>
void DisplayWrapper(T&& t) {
	Display(forward<T>(t));
}

TEST_CLASS(cppStudy)
{
TEST_METHOD(loger_perfectForward)
{
	Display(1);

	int x = 2;
	Display(x);

	int&& y = 7;
	y = 8;

	struct Player {
		string name;
	};

	struct Team {
		Team() {
			goalKeeper = new Player{"Marc" };
		};
		Team(const Team& t) {
			goalKeeper = new Player{ *t.goalKeeper };
		};
		Team(Team&& t) { // move constructor
			goalKeeper = t.goalKeeper;
			t.goalKeeper = nullptr;
		};
		~Team() { delete goalKeeper; }

		Player* goalKeeper;
	};

	DisplayWrapper(1);
	DisplayWrapper(x);
}
};

}
}

