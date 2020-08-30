#include "stdafx.h"
#include "CppUnitTest.h"
#include "ini.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;

using namespace std;

namespace Core
{
TEST_CLASS(File_)
{
public:
	TEST_METHOD(FileExt)
	{
		{
			string filePath = "c:\\dir\\test.txt";
			string ext = File::GetFileExt(filePath);

			Assert::IsTrue(ext == ".txt");
		}

		{
			string filePath = "c:/dir.tmp\\test.bin";
			string ext = File::GetFileExt(filePath);

			Assert::IsTrue(ext == ".bin");
		}
		{
			string filePath = "c:\\dir.txt\\test";
			string ext = File::GetFileExt(filePath);

			Assert::IsTrue(ext == "");
		}

	}


};
}
