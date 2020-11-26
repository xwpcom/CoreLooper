#pragma once
#include "libhttp/ajaxhandler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {


class Ajax_ListAjaxCommand :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_ListAjaxCommand)
protected:
	Ajax_ListAjaxCommand();
	virtual ~Ajax_ListAjaxCommand();

	string Process(const NameValue& params);
};
}
}
}
}
