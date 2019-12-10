#include "stdafx.h"
#include "unittest.h"
#include "CppUnitTest.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  

#ifdef _MSC_VER_DEBUG
//#define new DEBUG_NEW
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

namespace CoreLooper_Test
{
TEST_CLASS(Log)
{
	TEST_METHOD(SharedMemory_Reader)
	{
		int bytes = 1024 * 4*1024;
		auto fd=CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bytes, _T("Local\\CoreLooper"));
		//auto fd2 = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bytes, _T("Local\\CoreLooper"));
		//DV("fd=%p,fd2=%p", fd,fd2);

		auto d = (LPBYTE)MapViewOfFile(fd, FILE_MAP_READ,0,0, bytes);
		//d[bytes] = 0xCD;//test overflow,vs can detect it
		int x = 0;

	}
};

}



