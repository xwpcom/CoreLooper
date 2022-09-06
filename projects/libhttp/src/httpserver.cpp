#include "stdafx.h"
#include "httpserver.h"
#include "httphandler.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "HttpServer";

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
#if defined _CONFIG_OPENSSL && defined _MSC_VER
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
	handler->SetConfig(mWebConfig);

	handler->AttachChannel(endPoint);
	AddChild(handler);
	handler->OnConnect(endPoint, error, extraInfo);

	//LogV(TAG, "");

}


}
}
}
}