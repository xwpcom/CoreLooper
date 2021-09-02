#include "stdafx.h"
#include "CppUnitTest.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "include/rs485monitor.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

namespace UnitTest
{
TEST_CLASS(_rs485)
{
public:
	TEST_METHOD(parse)
	{
		RS485Monitor obj;
		obj.OnCreate();

		BYTE d[] = {
			0x01,0x02,
			0x1D,0x03,0x00,0x00,0x00,0x04,0x46,0x55,
			0x33,
			0x1D,0x03,0x08,0x00,0xFC,0x00,0x02,0x01,0x14,0x00,0x01,0x3B,0xB0,
			0x55,
		};
		
		int bytes = sizeof(d);
		obj.Input(d, bytes);
	}
};

}
