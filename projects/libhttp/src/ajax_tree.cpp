#include "stdafx.h"
#include "ajax_tree.h"

namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

IMPLEMENT_AJAX_CLASS(Ajax_Tree, "tree", "DeviceConfig")

Ajax_Tree::Ajax_Tree()
{
}

Ajax_Tree::~Ajax_Tree()
{
}

string Ajax_Tree::Process(const NameValue& params)
{
	string xml;
	LooperImpl* looper = Looper::GetMainLooper();
	if (!looper)
	{
		looper = Looper::CurrentLooper();
	}

	DWORD flags = 1;
	auto url = params.GetString("url");
	if (!url.empty())
	{
		auto obj = looper->FindObject(url);
		if (obj)
		{
			obj->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml, (LPARAM)(long)flags);
		}
	}
	else
	{
		looper->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml, (LPARAM)(long)flags);
	}

	string ack=StringTool::Format("<Result><Error>0</Error>%s</Result>"
		,xml.c_str()
	);

	return ack;
}

}
}
}
}
