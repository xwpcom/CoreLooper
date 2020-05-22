#pragma once
#include "ajaxhandler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

class Ajax_Tree :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_Tree)
protected:
	Ajax_Tree();
	virtual ~Ajax_Tree();

	string Process(const NameValue& params);
};

}
}
}
}
