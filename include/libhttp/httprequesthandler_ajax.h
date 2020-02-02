#pragma once
#include "httprequesthandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2013.06.13
class HTTP_EXPORT HttpRequestHandler_Ajax :public HttpRequestHandler
{
public:
	HttpRequestHandler_Ajax();
	virtual ~HttpRequestHandler_Ajax();

	virtual int Start(tagHttpHeaderInfo *headerInfo);
	virtual int Process();
protected:
};
}
}
}
}