#pragma once

namespace Windows {
using namespace std;
using namespace Bear::Core;

/*
XiongWanPing 2022.12.11
https://learn.microsoft.com/en-us/windows/win32/debug/debug-help-library
windows dbghelp wrapper
*/
class CORE_EXPORT DbgHelper
{
public:
	DbgHelper();

	int createMiniDump(const string& exeName,const string& dumpFilePath);
	int processId(const string& exeName);
	int test();

	string mTag = "DbgHelper";
};

}
