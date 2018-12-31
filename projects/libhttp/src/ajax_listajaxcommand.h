#pragma once
#include "libhttp/ajaxhandler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

//rtsp通道创建后可发送Login.xml来登录
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
