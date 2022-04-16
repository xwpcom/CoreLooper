#include "stdafx.h"
#include "CppUnitTest.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "libdb.inl"
#include "libdb/mysqlex.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;
using namespace Database;

namespace DB_Test
{
TEST_CLASS(DB_Test_Cases)
{
public:
	TEST_METHOD(Demo)
	{
		MySql obj;
		auto ret=obj.Connect("127.0.0.1", "root", "xxxxxx", "iot", 3306);
		DV("ret=%d", ret);
	}
};

}

