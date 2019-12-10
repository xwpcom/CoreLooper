#include "stdafx.h"
#include "memorylog.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

namespace Bear {
namespace Core {

MemoryLog::MemoryLog()
{
	DV("%s,this=%p", __func__, this);
}

MemoryLog::~MemoryLog()
{
	DV("%s,this=%p", __func__, this);
}

//
void MemoryLog::AddLog(const char *msg)
{
	DV("%p.%s(%s)", this,__func__,msg);
}

MemoryLog* MemoryLogD::GetInstance()
{
	static MemoryLog gDefaultLog;
	return &gDefaultLog;
}

MemoryLogEx::MemoryLogEx()
{
	DV("%s,this=%p", __func__, this);
}
MemoryLogEx::~MemoryLogEx()
{
	DV("%s,this=%p", __func__, this);
}

//static MemoryLogEx gLogEx;

MemoryLog* MemoryLogEx::GetInstance()
{
	/*
	//想让vs不报memory leak,试了几种方法都没成功
	{
		static bool first = true;
		if (first)
		{
			first = false;
			AfxEnableMemoryLeakDump(false);

			// Get current flag
			int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

			// Turn off leak-checking bit.
			tmpFlag &= ~_CRTDBG_LEAK_CHECK_DF;

			// Turn off CRT block checking bit.
			tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

			// Set flag to the new value.
			_CrtSetDbgFlag(tmpFlag);

		}
	}
	*/
	
	static MemoryLogEx* obj = new MemoryLogEx;

	/*
	{
		//not work,vs still report memory leaks 

		static bool first = true;
		if (first)
		{
			first = false;
			AfxEnableMemoryLeakDump(true);

			// Get current flag
			int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

			// Turn on leak-checking bit.
			tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

			// Turn off CRT block checking bit.
			tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;

			// Set flag to the new value.
			_CrtSetDbgFlag(tmpFlag);
		}
	}
	*/

	//atexit();//not work perfect at all case

	return obj;// &gLogEx;
}


}
}

#ifdef _DEBUG_APP
using namespace Bear::Core;
int main()
{
	class MainLooper :public MainLooper_
	{
		void OnCreate()
		{
			__super::OnCreate();

			auto p = new int;

			PostQuitMessage();
		}
	};
	
	
	class Helper
	{
	public:
		static void __cdecl Free()
		{
			TRACE("%s\r\n", __func__);
			_CrtDumpMemoryLeaks();
		}

		Helper()
		{
			atexit(Helper::Free);
		}
		~Helper()
		{
			TRACE("%s\r\n", __func__);
			atexit(Helper::Free);
		}
	};

	static Helper helper;
	

	make_shared<MainLooper>()->StartRun();

	return 0;
}
#endif
