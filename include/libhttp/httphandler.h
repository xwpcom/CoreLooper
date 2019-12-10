#pragma once
#include "looper/handler.h"
#include "libhttp/httpconfig.h"
namespace Bear {
namespace Core {
namespace Net {
class Channel;
namespace Http {
class HttpRequest;

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
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);

	void CheckSend();
	virtual void ParseInbox();
	
	virtual ByteBuffer* GetBuffer_ParseInbox()//便于https扩展
	{
		return &mInbox;
	}

	virtual ByteBuffer* GetOutbox()//便于https扩展
	{
		return &mOutbox;
	}

	virtual void TransformOutboxData() {}
	bool mCheckSending = false;

	ByteBuffer					mInbox;//缓存已接收但还没完成解析的数据
	ByteBuffer					mOutbox;
	std::shared_ptr<HttpRequest>	mHttpRequest;
	std::shared_ptr<tagWebServerConfig> mWebConfig;
};
}
}
}
}
