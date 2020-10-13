#include "stdafx.h"
#include "core/protocol/ctp/ctpserver2.h"
#include "core/protocol/ctp/CtpHandler2.h"


namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

CtpServer2::CtpServer2()
{
	SetObjectName("CtpServer2");
}

void CtpServer2::OnCreate()
{
	__super::OnCreate();

}

shared_ptr<Channel> CtpServer2::CreateChannel()
{
	auto obj(make_shared<TcpClient>());
	obj->SignalOnConnect.connect(this, &CtpServer2::OnConnect);
	return obj;
}

shared_ptr<CtpHandler2> CtpServer2::CreateHandler()
{
	auto handler(make_shared<CtpHandler2>());
	return handler;
}

void CtpServer2::OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo)
{
	TcpClient* client = (TcpClient*)endPoint;
	client->SignalOnConnect.disconnect(this);

	auto handler(CreateHandler());
	ASSERT(handler);
	handler->mDataEndPoint = dynamic_pointer_cast<TcpClient>(client->shared_from_this());

	client->SignalOnSend.connect(handler.get(), &CtpHandler2::OnSend);
	client->SignalOnReceive.connect(handler.get(), &CtpHandler2::OnReceive);
	client->SignalOnClose.connect(handler.get(), &CtpHandler2::OnClose);

	AddChild(handler);

	handler->OnConnect(endPoint, error, extraInfo);
}

}
}
}
}
}
