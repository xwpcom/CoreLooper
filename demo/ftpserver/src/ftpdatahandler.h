#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSource;
class DataSink;

class FtpDataHandler :public Handler
{
	friend class FtpDataServer;
	SUPER(Handler);
public:
	FtpDataHandler();
	~FtpDataHandler();

	shared_ptr<Channel> mDataEndPoint;

	virtual void SetDataSource(shared_ptr<DataSource> obj);
	virtual void SetDataSink(shared_ptr<DataSink> obj);
	virtual void CheckSend();

	void OnDataChanged(DataSource *source);

	sigslot::signal1<FtpDataHandler*> SignalSendDone;
	sigslot::signal1<FtpDataHandler*> SignalSinkDone;
protected:
	void OnConnect(Channel*, long error, Bundle*);
	void OnSend(Channel*);
	void OnReceive(Channel*);
	void OnClose(Channel*);

	weak_ptr<DataSource>	mDataSource;
	weak_ptr<DataSink>		mDataSink;

	ByteBuffer mOutbox;
	bool mHasFireSendDone = false;
	bool mSinkDoneFired = false;
	bool mDataOut = true;
};
}
}
}
}