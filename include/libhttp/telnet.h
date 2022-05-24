#pragma once
#include "net/simpleconnect.h"

namespace Bear {
namespace Telnet {
using namespace Bear::Core;
using namespace Bear::Core::Net;

/*
XiongWanPing 2022.05.24
为方便远程管理iot设备，需要用到反向telnet
*/

class TelnetHandler;

class HTTP_EXPORT TelnetServer:public TcpServer
{
	SUPER(TcpServer);
public:
	TelnetServer();
	int setUid(const string& uid);
protected:
	void OnCreate();
	void OnTimer(long id);

	virtual shared_ptr<Channel> CreateChannel();
	virtual shared_ptr<TelnetHandler> CreateHandler();
	virtual void OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);

	string mTag = "telnetServer";
	string mUid;
	long mTimer_uidTimeout = 0;
};

class HTTP_EXPORT TelnetClient :public SimpleConnect
{
	SUPER(SimpleConnect);
public:
	TelnetClient();
protected:
	void OnCreate();

	string mTag = "telnetClient";
};


}
}
