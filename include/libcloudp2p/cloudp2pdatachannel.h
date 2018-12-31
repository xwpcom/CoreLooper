#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {
class CloudP2PConnecter;

//XiongWanPing 2016.07.15
class CLOUD_P2P_EXT_CLASS CloudP2PDataChannel :public Channel
{
	SUPER(Channel);
public:
	CloudP2PDataChannel();
	virtual ~CloudP2PDataChannel();
	void SetConnecter(std::weak_ptr<CloudP2PConnecter>connecter)
	{
		mConnecter = connecter;
	}

	void AttachSid(int sid);

	virtual int Connect(Bundle& info);

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	void OnCreate();
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	virtual void Close_Impl();

	virtual int OnConnect(long handle, Bundle* extraInfo);
	virtual void OnReceive();
	virtual void OnSend();
	virtual void OnClose();

	void OnTimer(long timerId);
	void Process();

protected:
	std::weak_ptr<CloudP2PConnecter>	mConnecter;
	bool mNeedCheckOutbox;
	int  mSid;
	bool mSignalCloseHasFired;

	std::string mMode;//relay or direct
	std::string mCreateTime;
	long mTimerProcess = 0;
};
}
}
}
}