#pragma once
#include "looper/handler.h"

namespace Bear {
namespace Core {
using namespace FileSystem;

namespace Crypt {
class TlsProxy;
}
namespace Net {
class Channel;
namespace Https {

//XiongWanPing 2018.06.09
class DemoHttps :public Handler
{
	SUPER(Handler)
public:
	DemoHttps();
	virtual ~DemoHttps();

	shared_ptr<Channel> mChannel;

	virtual void OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

protected:
	void OnCreate();

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	void ParseInbox();
	void OnRecvHttpAckBody(LPVOID data, int dataLen);
	void SendRequest();
	void CheckSend();
	void OnTimer(long timerId);

	DumpFile mDumpFile;

	enum eHttpAckStatus
	{
		eHttpAckStatus_WaitHeader,
		eHttpAckStatus_ReceivingBody,
		eHttpAckStatus_Done,
	};
	void SwitchStatus(eHttpAckStatus status);

	struct tagHttpAckInfo
	{
		tagHttpAckInfo()
		{
			Reset();
		}

		void Reset()
		{
			mHttpAckStatus = eHttpAckStatus_WaitHeader;
			mContentLength = 0;
			mContentRecvBytes = 0;
		}

		eHttpAckStatus	mHttpAckStatus;
		DWORD			mContentLength;
		DWORD			mContentRecvBytes;
	}mHttpAckInfo;

	ByteBuffer mInbox, mOutbox;
	int mReqIndex = 0;
	long mTimerDelayExit = 0;
	shared_ptr<Crypt::TlsProxy> mProxy;
};
}
}
}
}
