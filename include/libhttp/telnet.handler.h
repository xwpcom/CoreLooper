#pragma once
#include "core/net/channel.h"

namespace Bear {
namespace Telnet {
using namespace Bear::Core;
using namespace Bear::Core::Net;

class HTTP_EXPORT TelnetHandler :public Handler
{
	SUPER(Handler);
	friend class TelnetServer;
public:
	TelnetHandler();
	~TelnetHandler();

	shared_ptr<Channel> mDataEndPoint;
protected:
	void OnCreate();
	void OnTimer(long id);
	virtual void OnProtocolCreated() {}

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	void CheckSend();

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;

	ByteBuffer	mOutbox;
	long mTimer_CheckAlive = 0;
};

}
}
