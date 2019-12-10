#pragma once
#include "httprequesthandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2019.11.30
//处理ajax .xml命令
class HTTP_EXPORT HttpRequestHandler_Json :public HttpRequestHandler
{
public:
	HttpRequestHandler_Json();
	virtual ~HttpRequestHandler_Json();

	virtual int Start(tagHttpHeaderInfo* headerInfo);
	virtual int Process();
protected:
};

}
}
}
}
