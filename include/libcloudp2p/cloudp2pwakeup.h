#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {

//XiongWanPing 2016.12.16
/*
mtk6572 3g ipcam进入待机后，电流可低至7ma,此时arm主芯片休眠，只有基带芯片在侦听wifi/3g信号
待机1分钟之内，外网可通过p2p连接上ipcam,
超时1分钟，则连接不上
所以需要采用措施来让p2p保活
尚云的解决办法是:
每5分钟向p2p服务器发一个保活包
*/
class CLOUD_P2P_EXT_CLASS CloudP2PWakeup :public Handler
{
#ifndef _MSC_VER
	typedef Handler __super;
#endif

public:
	CloudP2PWakeup();
	~CloudP2PWakeup();

	void ResetStatus()
	{
		mP2POK = false;
	}
	int KeepAlive();
	void Reconnect();

protected:
	void OnCreate();

	virtual void OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	int SendData(LPVOID data, int dataBytes);
	void WakeupDevice(int seconds);

	void OnTimer(long timerId);

	void CreateConnect();

	void OnP2PStatusChanged(bool online);

	std::shared_ptr<Channel> mChannel;
	std::string mKeepAliveData;
	bool mConnected;

	int mRecvBytes;
	int mSendBytes;

	bool mP2POK;//仅在p2p在线并且保活正常时，才允许待机
};

}
}
}
}
