#include "stdafx.h"
#include "ftpserver.h"
#include "net/tcpclient.h"
#include "ftpcommandhandler.h"

using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpServer::FtpServer()
{

}

void FtpServer::OnCreate()
{
	__super::OnCreate();
}

shared_ptr<Channel> FtpServer::CreateChannel()
{
	ASSERT(IsMyselfThread());

	auto client(make_shared<TcpClient>());
	client->SignalOnConnect.connect(this, &FtpServer::OnConnect);
	return client;
}

void FtpServer::OnConnect(Channel* endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo)
{
	endPoint->SignalOnConnect.disconnect(this);
	HandleConnect(dynamic_pointer_cast<Channel>(endPoint->shared_from_this()), error, nullptr, extraInfo);
}

void FtpServer::HandleConnect(shared_ptr<Channel> endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo)
{
	ASSERT(IsMyselfThread());
#if 1
	auto handler(make_shared<FtpCommandHandler>());
	handler->SetConfig(mConfig);
	handler->mDataEndPoint = endPoint;

	endPoint->SignalOnSend.connect(handler.get(), &FtpCommandHandler::OnSend);
	endPoint->SignalOnReceive.connect(handler.get(), &FtpCommandHandler::OnReceive);
	endPoint->SignalOnClose.connect(handler.get(), &FtpCommandHandler::OnClose);

	//handler->SetConfig(mRtspConfig);
	AddChild(handler);

	handler->OnConnect(endPoint.get(), error, extraInfo);
#endif
}
