#include "stdafx.h"
#include "httpserver.h"
#include "httphandler.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpServer::HttpServer()
{
	SetObjectName("HttpServer");
}

HttpServer::~HttpServer()
{
}

std::shared_ptr<Channel> HttpServer::CreateChannel()
{
	auto client(make_shared<Net::TcpClient>());
#ifdef _CONFIG_OPENSSL
	if (mUseTls)
	{
		client->EnableTls();
	}
#endif
	client->SignalOnConnect.connect(this, &HttpServer::OnConnect);
	return client;
}

void HttpServer::OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle*extraInfo)
{
	TcpClient* client = (TcpClient*)endPoint;
	client->SignalOnConnect.disconnect(this);

	auto handler(make_shared<HttpHandler>());
	handler->mChannel = dynamic_pointer_cast<TcpClient>(client->shared_from_this());
	handler->SetConfig(mWebConfig);

	client->SignalOnSend.connect(handler.get(), &HttpHandler::OnSend);
	client->SignalOnReceive.connect(handler.get(), &HttpHandler::OnReceive);
	client->SignalOnClose.connect(handler.get(), &HttpHandler::OnClose);

	AddChild(handler);
	handler->OnConnect(endPoint, error, extraInfo);
}


}
}
}
}