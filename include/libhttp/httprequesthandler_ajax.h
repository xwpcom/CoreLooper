#pragma once
#include "httprequesthandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2013.06.13
//处理ajax .xml命令
class HTTP_EXPORT CHttpRequestHandler_Ajax :public HttpRequestHandler
{
public:
	CHttpRequestHandler_Ajax();
	virtual ~CHttpRequestHandler_Ajax();

	virtual int Start(tagHttpHeaderInfo *headerInfo);
	virtual int Process();
protected:
};
}
}
}
}