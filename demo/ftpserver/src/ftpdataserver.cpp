#include "stdafx.h"
#include "ftpdataserver.h"
#include "ftpdatainhandler.h"
#include "ftpdataouthandler.h"
#include "net/tcpclient.h"

using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpDataServer::FtpDataServer()
{
	SetObjectName("FtpDataServer");
	DW("%s(%p)", __func__, this);
}

FtpDataServer::~FtpDataServer()
{
	DW("%s(%p)", __func__, this);
}

shared_ptr<Channel> FtpDataServer::CreateChannel()
{
	ASSERT(IsMyselfThread());

	auto client(make_shared<TcpClient>());
	client->SignalOnConnect.connect(this, &FtpDataServer::OnConnect);
	return client;
}

/*
ftp协议没有严格规定client发送PASV后的动作时序,导致FtpDataServer收到连接时,FtpCommandHandler可能还没收到LIST,STOR或RETR命令
这样FtpDataServer没法知道新创建的连接是in还是out,只能延时处理
*/
void FtpDataServer::OnConnect(Channel* endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo)
{
	endPoint->SignalOnConnect.disconnect(this);

	auto channel = dynamic_pointer_cast<Channel>(endPoint->shared_from_this());
	if (mMode != eFtpDataMode_None)
	{
		HandleConnect(channel, error, nullptr, extraInfo);
	}
	else
	{
		//FtpCommandHandler还没收到LIST,STOR或RETR命令，这里缓存
		//稍后在FtpDataServer::SetMode中处理
		mChannelCache = channel;
	}
}

void FtpDataServer::HandleConnect(shared_ptr<Channel> endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo)
{
	ASSERT(IsMyselfThread());
	//ASSERT(mMode != eFtpDataMode_None);
	DV("FtpDataServer(%p)::HandleConnect",this);

	shared_ptr<FtpDataHandler> handler;
	if (mMode == eFtpDataMode_In)
	{
		handler = make_shared<FtpDataInHandler>();
	}
	else if (mMode == eFtpDataMode_Out)
	{
		handler = make_shared<FtpDataOutHandler>();
	}
	else
	{
		ASSERT(FALSE);
		DW("handler is null");
		return;
	}

	handler->mDataEndPoint = endPoint;

	endPoint->SignalOnSend.connect(handler.get(), &FtpDataHandler::OnSend);
	endPoint->SignalOnReceive.connect(handler.get(), &FtpDataHandler::OnReceive);
	endPoint->SignalOnClose.connect(handler.get(), &FtpDataHandler::OnClose);

	AddChild(handler);
	handler->OnConnect(endPoint.get(), error, extraInfo);

	SignalOnConnect(handler);

	SetMode(eFtpDataMode_None);
}

void FtpDataServer::SetMode(eFtpDataMode mode)
{
	static const char *arr[] = {"none","in","out",};

	if (mode >= 0 && mode < COUNT_OF(arr))
	{
		DW("FtpDataServer(%p).SetMode(%s)", this, arr[mode]);
	}
	else
	{
		ASSERT(FALSE);
	}

	mMode = mode;

	if (mMode != eFtpDataMode_None)
	{
		auto obj = mChannelCache.lock();

		if (obj)
		{
			mChannelCache.reset();
			HandleConnect(obj, 0, nullptr, nullptr);
		}
	}
}
