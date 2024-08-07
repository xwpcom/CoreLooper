﻿#pragma once
//XiongWanPing 2016.03.25
#include "net/tcpserver.h"
#include "base/sigslot.h"
#include "httpconfig.h"
namespace Bear {
namespace Core {
class Bundle;
namespace Net {
class Channel;
namespace Http {
class HTTP_EXPORT HttpServer :public TcpServer
{
public:
	HttpServer();
	virtual ~HttpServer();

	virtual void SetConfig(std::shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}
	virtual shared_ptr<tagWebServerConfig> GetConfig()const
	{
		return mWebConfig;
	}

protected:
	void OnConnect(Channel*, long, ByteBuffer*, Bundle*);
	virtual std::shared_ptr<Channel> CreateChannel();

	std::shared_ptr<tagWebServerConfig> mWebConfig;
};
}
}
}
}