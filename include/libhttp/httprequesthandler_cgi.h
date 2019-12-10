#pragma once
#include "httprequesthandler.h"
#include "file/FileFinder.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {
//XiongWanPing 2013.06.13
//处理.cgi命令
class HTTP_EXPORT HttpRequestHandler_CGI :public HttpRequestHandler
{
public:
	HttpRequestHandler_CGI();
	virtual ~HttpRequestHandler_CGI();

	virtual int Start(tagHttpHeaderInfo *headerInfo);
	virtual int Process();

protected:
	virtual int	ProcessCgi(std::string & cgiAck);
};
}
}
}
}