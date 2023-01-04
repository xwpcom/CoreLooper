#include "stdafx.h"
#include "CppUnitTest.h"
#include "loger2.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core2;


namespace Bear {
namespace Core2 {
static const char* TAG = "log";

TEST_CLASS(Log_)
{
	TEST_METHOD(loger)
	{
		//LogerV(TAG)<<"hello";
		//LogerV << "hello";
		int year = 2023;
		TraceL << u8"hello"<<" world "<<year<<u8" 新年快乐! ";
		TraceL << u8"second line";
	}
};

}
}

