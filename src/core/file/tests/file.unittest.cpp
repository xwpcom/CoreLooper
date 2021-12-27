#include "stdafx.h"
#include "CppUnitTest.h"
#include "ini.h"
#include "core/looper/teststate.h"
#include "core/net/tcpclient.h"
#include <atomic> 
#include <functional>
#include <mutex>  
#include "string/utf8tool.h"

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
static const char* TAG = "file";

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

	TEST_METHOD(findFiles)
	{
		string filter = R"(D:\corelooper\projects\iot\bin\media\hello)";

		FileFinder finder;
		BOOL bOK = finder.FindFile(filter);
		//LogV(TAG, "FindFile(%s),ok=%d", filter.c_str(),bOK);

		while (bOK)
		{
			bOK = finder.FindNextFile();
			if (finder.IsDots())
			{
				continue;
			}

			auto file = finder.GetFileName();
			LogV(TAG, "%s", file.c_str());
		}
	}

	TEST_METHOD(openFile)
	{
		string szFileName = u8"D:\\corelooper\\projects\\iot\\bin\\media\\中.jpg";
		auto file = Utf8Tool::UTF_8ToGB2312(szFileName);
		auto hFile = File::fopen(file.c_str(), "rb");
		fclose(hFile);

	}


};
}
