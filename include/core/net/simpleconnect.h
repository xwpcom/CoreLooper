#pragma once
namespace Bear {
namespace Core
{
namespace Net {

class Channel;
//XiongWanPing 2018.07.11
//经常需要用到tcp短连接，可从SimpleConnect继承
class CORE_EXPORT SimpleConnect :public Handler
{
	SUPER(Handler)
public:
	SimpleConnect();
	virtual ~SimpleConnect();

	int StartConnect(Bundle& bundle);
	void DelayAutoClose(int ms);
	void KeepAlive()
	{
		if (mTimeOutSecond > 0)
		{
			DelayAutoClose(mTimeOutSecond * 1000);
		}
	}

	virtual void ParseInbox()
	{
		//正常情况下，应该在子类及时取出并处理
		//当子类没有处理时，在这里自动删除过长的数据,防止占用过多内存
		if (mInbox.GetDataLength() > 64 * 1024)
		{
			mInbox.clear();
		}
	}

	virtual void OnOutboxWritable() {};
	virtual bool IsConnected()const
	{
		return mConnected;
	}

	sigslot::signal3<SimpleConnect*, LPBYTE, int> SignalSendOut;//可用来监测已提交的数据

protected:
	virtual void OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);
	void OnDestroy();
	void OnTimer(UINT id);

	void CheckSend();
	void Close();

	shared_ptr<Channel> mDataEndPoint;
	bool mConnected = false;
	int mTimeOutSecond = 90;
	ByteBuffer mInbox;
	ByteBuffer mOutbox;

	long mTimer_AutoClose = 0;
};

}
}
}
