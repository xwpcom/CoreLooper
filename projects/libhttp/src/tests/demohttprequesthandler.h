#pragma once
#include "looper/handler.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
namespace Net {
class Channel;
namespace Http {

//XiongWanPing 2016.03.19
//用来模拟http request客户端，测试TcpClient机制在客户端的应用场景
class HTTP_EXPORT DemoHttpRequestHandler :public Handler
{
	SUPER(Handler)
public:
	DemoHttpRequestHandler();
	virtual ~DemoHttpRequestHandler();

	std::shared_ptr<Channel> mChannel;

	virtual void OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

protected:
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	void ParseInbox();
	void OnRecvHttpAckBody(LPVOID data, int dataLen);
	void SendRequest();
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
	long mTimerCheckSend = 0;
};
}
}
}
}
