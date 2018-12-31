#pragma once
#include "looper/handler.h"
#include "libhttp/httpconfig.h"
namespace Bear {
namespace Core {
namespace Net {
class Channel;
namespace Http {
class CHttpRequest;

//XiongWanPing 2016.03.25
//处理一个http socket连接
//代码来自cs::CHttpClient
class HTTP_EXPORT HttpHandler :public Handler
{
	friend class HttpServer;
public:
	HttpHandler();
	virtual ~HttpHandler();

	virtual void SetConfig(std::shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}

	std::shared_ptr<Channel> mChannel;
protected:
	void CheckSend();

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	ByteBuffer					mInbox;//缓存已接收但还没完成解析的数据
	ByteBuffer					mOutbox;
	std::shared_ptr<CHttpRequest>	mHttpRequest;
	std::shared_ptr<tagWebServerConfig> mWebConfig;
};
}
}
}
}
