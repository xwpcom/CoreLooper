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
	
	void setBuddy(weak_ptr< TelnetHandler> wobj);
	int sendData(LPBYTE data, int bytes);

	shared_ptr<Channel> mDataEndPoint;
protected:
	void OnCreate();
	void OnDestroy();
	void OnTimer(long id);

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	void CheckSend();
	void ParseInbox();
	void checkSendCacheData();
	bool IsConnected()const
	{
		return mConnected;
	}

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;

	ByteBuffer mInbox;
	ByteBuffer mInboxCache;/* 缓存在buddy连接成功之前收到的数据  */

	ByteBuffer	mOutbox;
	long mTimer_CheckAlive = 0;

	weak_ptr< TelnetHandler> mBuddy;
	string mTag = "TelnetHandler";
	bool mConnected = false;
};

}
}
