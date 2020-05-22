#pragma once
#include "ajaxhandler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

class Ajax_Proc :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_Proc)
protected:
	Ajax_Proc();
	virtual ~Ajax_Proc();

	string Process(const NameValue& params);
};

}
}
}
}
