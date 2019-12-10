#pragma once
#include "net/channel.h"
#include "net/speedcounter.h"

namespace Bear {
namespace Test {

using namespace Bear::Core;
using namespace Bear::Core::Net;

class SpeedHandler :public Handler
{
	SUPER(Handler);
	friend class SpeedServer;
public:
	SpeedHandler();
	~SpeedHandler();

	shared_ptr<Channel> mChannel;
protected:
	void OnCreate();
	void OnTimer(long id);

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	void CheckSend();

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;

	ByteBuffer		mOutbox;
	SpeedCounter	mCounterUp;		//上行计速
	SpeedCounter	mCounterDown;	//下行计速

	long mTimerDump = 0;
	long mTimerSend=0;
};

}
}