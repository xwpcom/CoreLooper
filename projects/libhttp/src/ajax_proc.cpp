#include "stdafx.h"
#include "ajax_proc.h"

using namespace Bear::Core;
HTTP_EXPORT void _avoidCompileRemove_proc()
{
	LogV("compile", "%s",__func__);
}

static char* TAG = "proc";

namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

IMPLEMENT_AJAX_CLASS(Ajax_Proc, "proc", "")

Ajax_Proc::Ajax_Proc()
{
}

Ajax_Proc::~Ajax_Proc()
{
}

string Ajax_Proc::Process(const NameValue& params)
{
	Profiler profile("Ajax_Proc");

	string xml;
	LooperImpl* looper = Looper::GetMainLooper();
	if (!looper)
	{
		looper = Looper::CurrentLooper();
	}

#if defined _CONFIG_PROFILER
	ULONGLONG  tick = 0;
	if (Looper::CurrentLooper()->profilerEnabled())
	{
		auto obj = Looper::CurrentLooper()->profiler();
		obj->procCallCount++;
		tick = ShellTool::GetTickCount64();
	}
#endif

	auto url = params.GetString("url");
	if (!url.empty())
	{
		auto obj = looper->FindObject(url);
		if (obj)
		{
			obj->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml);
		}
	}
	else
	{
		looper->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml);
	}

	string ack=StringTool::Format("<Result><Error>0</Error>%s</Result>"
		,xml.c_str()
	);


#if defined _CONFIG_PROFILER
	if (tick && Looper::CurrentLooper()->profilerEnabled())
	{
		tick = ShellTool::GetTickCount64() - tick;

		auto obj = Looper::CurrentLooper()->profiler();
		if (tick > obj->procMaxTick)
		{
			obj->procMaxTick = tick;
		}
	}

#endif

	return ack;
}

}
}
}
}
