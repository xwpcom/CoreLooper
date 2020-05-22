#include "stdafx.h"
#include "ajax_listajaxcommand.h"
#include "libhttp/ajaxhandler.h"

namespace Bear {

namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

IMPLEMENT_AJAX_CLASS(Ajax_ListAjaxCommand, "ListAjaxCommand", "")

Ajax_ListAjaxCommand::Ajax_ListAjaxCommand()
{
	//DV("%s", __func__);
}

Ajax_ListAjaxCommand::~Ajax_ListAjaxCommand()
{
	//DV("%s", __func__);
}

string Ajax_ListAjaxCommand::Process(const NameValue& params)
{
	auto arr = AjaxRuntimeClass::m_mapPackClass;
	string item, items;
	if (AjaxRuntimeClass::m_mapPackClass)
	{
		for (auto iter = AjaxRuntimeClass::m_mapPackClass->begin(); iter != AjaxRuntimeClass::m_mapPackClass->end(); ++iter)
		{
			item=StringTool::Format(
				"<AjaxInterface>"
				"<Command>%s</Command>"
				"<Permission>%s</Permission>"
				"</AjaxInterface>"
				, iter->second->mCommandName.c_str()
				, iter->second->mPermission.c_str()
			);
			items += item;
		}
	}
	//*/

	string ack=StringTool::Format("<Result><Error>0</Error>%s</Result>"
		,items.c_str()
	);
	return ack;
}

}
}
}
}
