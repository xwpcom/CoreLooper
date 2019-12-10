#include "stdafx.h"
#include "speedserver.h"
#include "net/tcpclient.h"
#include "speedhandler.h"

namespace Bear {
namespace Test {

SpeedServer::SpeedServer()
{

}

void SpeedServer::OnCreate()
{
	__super::OnCreate();

}

shared_ptr<Channel> SpeedServer::CreateChannel()
{
	auto client(make_shared<TcpClient>());
	client->SignalOnConnect.connect(this, &SpeedServer::OnConnect);
	return client;
}

void SpeedServer::OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle*extraInfo)
{
	//DV("%s", __func__);

	auto client = endPoint;
	client->SignalOnConnect.disconnect(this);

	auto handler(make_shared<SpeedHandler>());
	handler->mChannel = dynamic_pointer_cast<Channel>(client->shared_from_this());
	//handler->SetConfig(mWebConfig);

	client->SignalOnSend.connect(handler.get(), &SpeedHandler::OnSend);
	client->SignalOnReceive.connect(handler.get(), &SpeedHandler::OnReceive);
	client->SignalOnClose.connect(handler.get(), &SpeedHandler::OnClose);

	AddChild(handler);

	handler->OnConnect(endPoint, error, extraInfo);
}

}
}
