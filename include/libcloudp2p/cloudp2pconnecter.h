#pragma once
#include <atomic> 

namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {

enum
{
	BM_P2P_CONNECT_ACK,
};

struct tagRequestInfo
{
	std::string				mUid;
	std::weak_ptr<Handler>	mRequestHandler;
	UINT					mMsg;		//连接成功或失败时向mRequestHandler投递mMsg消息
};

//XiongWanPing 2016.07.16
//cloud p2p的connect是串行阻塞的，所以采用单独的looper来做连接
//2016.09.06 改动:
//当有多个设备时，如果其中一个设备不在线，连接会占用几秒钟才超时失败,这会导致其他设备的连接也会比较慢
//所以改为每个CloudP2PDataChannel创建自己的CloudP2PConnecter,连接一次后即销毁CloudP2PConnecter
class CLOUD_P2P_EXT_CLASS CloudP2PConnecter :public Looper
{
	SUPER(Looper);
public:
	CloudP2PConnecter();
	virtual ~CloudP2PConnecter();

	int SetConnectInfo(std::string uid, std::weak_ptr<Handler> handler, UINT msg);
	int CancelConnect();

protected:
	void OnCreate();
	int CreateConnectLooper(bool p2p);
	void OnTimer(long id);

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

protected:
	tagRequestInfo	mInfo;
	bool			mCancel = false;
	bool			mConnecting = false;
	bool			mAcked = false;
	bool			mEnableRelayLooper = false;
	bool			mRelayLooperCreated = false;
	int				mAckTimes = 0;
	
	long mTimerDelayRelay = 0;
	long mTimerDelayQuit = 0;
};
}
}
}
}