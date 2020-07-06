#include "stdafx.h"
#include "core/protocol/ctp/ctpserver.h"
#include "core/protocol/ctp/CtpHandler.h"


namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

CtpServer::CtpServer()
{
	SetObjectName("CtpServer");
}

void CtpServer::OnCreate()
{
	__super::OnCreate();

}

shared_ptr<Channel> CtpServer::CreateChannel()
{
	auto obj(make_shared<TcpClient>());
	obj->SignalOnConnect.connect(this, &CtpServer::OnConnect);
	return obj;
}

shared_ptr<CtpHandler> CtpServer::CreateHandler()
{
	auto handler(make_shared<CtpHandler>());
	return handler;
}

void CtpServer::OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo)
{
	TcpClient* client = (TcpClient*)endPoint;
	client->SignalOnConnect.disconnect(this);

	auto handler(CreateHandler());
	ASSERT(handler);
	handler->mDataEndPoint = dynamic_pointer_cast<TcpClient>(client->shared_from_this());

	client->SignalOnSend.connect(handler.get(), &CtpHandler::OnSend);
	client->SignalOnReceive.connect(handler.get(), &CtpHandler::OnReceive);
	client->SignalOnClose.connect(handler.get(), &CtpHandler::OnClose);

	AddChild(handler);

	handler->OnConnect(endPoint, error, extraInfo);
}

}
}
}
}
}
