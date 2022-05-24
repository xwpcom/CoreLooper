#include "stdafx.h"
#include "telnet.h"
#include "core/net/tcpclient.h"
#include "libhttp/telnet.handler.h"

namespace Bear {
namespace Telnet {
using namespace Bear::Core::Net;

/*
反向telnet工作概况
iotDevice-----------> iotPlatform 保持tcp长连接
secureCRT-----------> iotPlatform(把telnet连接透传给iotDevice)

细节
.secureCRT用telnet连接(sucureCRT2Iot)时只能提供ip port,所以需要用其他办法来告知需要连接的设备uid
.iotPlatform通知iotDevice有新telnet连接请求
.iotDevice创建一个本地连接到自己的telnetd(称为deviceTelnetClient),再创建一个新连接(device2Iot)到iotPlatform并表示身份
.现在就有如下三个连接:
 sucureCRT2Iot
 localTelnetClient
 device2Iot
.device负责转发deviceTelnetClient和device2Iot
 iot负责转发sucureCRT2Iot和device2Iot
*/

TelnetServer::TelnetServer()
{
	SetObjectName("TelnetServer");
}

void TelnetServer::OnCreate()
{
	__super::OnCreate();
}

void TelnetServer::OnTimer(long id)
{
	if (id == mTimer_uidTimeout)
	{
		KillTimer(mTimer_uidTimeout);

		LogW(mTag, "uid[%s] timeout",mUid.c_str());
		mUid.clear();

		return;
	}

	__super::OnTimer(id);
}

int TelnetServer::setUid(const string& uid)
{
	/*
	每次只能绑定一个uid
	*/

	if (mUid.empty())
	{
		mUid = uid;
		LogV(mTag, "%s(%s) ok", __func__, uid.c_str());

		SetTimer(mTimer_uidTimeout, 10 * 1000);
		return 0;
	}

	LogW(mTag, "fail %s(%s) due to busying",__func__,uid.c_str());
	return -1;
}

shared_ptr<Channel> TelnetServer::CreateChannel()
{
	auto obj(make_shared<TcpClient>());
	obj->SignalOnConnect.connect(this, &TelnetServer::OnConnect);
	return obj;
}

shared_ptr<TelnetHandler> TelnetServer::CreateHandler()
{
	auto handler(make_shared<TelnetHandler>());
	return handler;
}

void TelnetServer::setBuddy(weak_ptr< TelnetServer> wobj)
{
	mBuddy = wobj;

	auto obj = wobj.lock();
	if (obj)
	{
		
	}
}

void TelnetServer::onBuddyConnectReady(shared_ptr<TelnetHandler> obj)
{
	auto buddy = mBuddy.lock();
	if (buddy)
	{
		auto con = fetchCacheHandler();
		if (con)
		{
			obj->setBuddy(con);
			con->setBuddy(obj);
		}
	}
}

void TelnetServer::OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo)
{
	if (deviceMode())
	{
		//from device master
		int x = 0;

	}
	else
	{
		//from secureCRT
		int x = 0;
	}

	TcpClient* client = (TcpClient*)endPoint;
	client->SignalOnConnect.disconnect(this);

	auto handler(CreateHandler());
	ASSERT(handler);
	handler->mDataEndPoint = dynamic_pointer_cast<TcpClient>(client->shared_from_this());

	client->SignalOnSend.connect(handler.get(), &TelnetHandler::OnSend);
	client->SignalOnReceive.connect(handler.get(), &TelnetHandler::OnReceive);
	client->SignalOnClose.connect(handler.get(), &TelnetHandler::OnClose);

	AddChild(handler);

	handler->OnConnect(endPoint, error, extraInfo);

	mCacheHandler = handler;

	{
		auto buddy = mBuddy.lock();
		if (buddy)
		{
			buddy->onBuddyConnectReady(handler);
		}
	}
}

}
}
