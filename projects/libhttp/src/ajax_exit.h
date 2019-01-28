#pragma once
#include "ajaxhandler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

class Ajax_Exit :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_Exit)
protected:
	Ajax_Exit();
	virtual ~Ajax_Exit();

	string Process(const NameValue& params);
};

}
}
}
}
