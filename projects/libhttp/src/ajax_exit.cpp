#include "stdafx.h"
#include "ajax_exit.h"

namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

IMPLEMENT_AJAX_CLASS(Ajax_Exit, "exit", "DeviceConfig")

Ajax_Exit::Ajax_Exit()
{
}

Ajax_Exit::~Ajax_Exit()
{
}

string Ajax_Exit::Process(const NameValue& params)
{
	string xml;
	int error = -1;

#ifdef _DEBUG
	//本接口仅供测试使用
	LooperImpl* looper = Looper::GetMainLooper();
	if (!looper)
	{
		looper = Looper::CurrentLooper();
	}

	if (looper)
	{
		looper->postDelayedRunnable(make_shared<DelayExitRunnable>(), 3 * 1000);
		error = 0;
	}
#endif

	string ack = StringTool::Format("<Result><Error>%d</Error>%s</Result>"
		,error
		, xml.c_str()
	);

	return ack;
}

}
}
}
}