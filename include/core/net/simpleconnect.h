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

	void SetTimeout(int seconds);
	
	void EnableVerbose()
	{
		mVerbose = true;
	}

	const string& address()const
	{
		return mAddress;
	}

	void DisableVerbose()
	{
		mVerbose = false;
	}

#ifdef _MSC_VER
	//目前只有windows下做了openssl支持
	void EnableTls()
	{
		mUseTls = true;
	}
#endif

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
	sigslot::signal2<Handler*,int> SignalConnectAck;
	sigslot::signal1<Handler*> SignalDestroy;
protected:
	virtual void OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);
	void OnDestroy();
	void OnTimer(long id);

	void CheckSend();
	void Close();

	shared_ptr<Channel> mChannel;//todo:采用weak_ptr,或者把channel移到其他节点下,避免强引用child
	bool mConnected = false;
	int mTimeOutSecond = 90;
	ByteBuffer mInbox;
	ByteBuffer mOutbox;
	string mAddress;

	long mTimer_AutoClose = 0;

#ifdef _MSC_VER
	bool mUseTls = false;
#endif

	bool mVerbose = false;
};

}
}
}
