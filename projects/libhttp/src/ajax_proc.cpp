#include "stdafx.h"
#include "ajax_proc.h"

using namespace Bear::Core;
HTTP_EXPORT void _avoidCompileRemove_proc()
{
	LogV("compile", "%s",__func__);
}

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
	string xml;
	LooperImpl* looper = Looper::GetMainLooper();
	if (!looper)
	{
		looper = Looper::CurrentLooper();
	}

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

#ifdef _DEBUG
	File::Dump(ack, "d:/proc.xml");
#endif

	return ack;
}

}
}
}
}
