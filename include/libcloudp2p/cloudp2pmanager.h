#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P{

enum
{
	BM_CONNECT_ACK,
	BM_P2P_ACCEPT,
};

//XiongWanPing 2016.07.14
class CLOUD_P2P_EXT_CLASS CloudP2PManager :public Handler
{
	SUPER(Handler)
public:
	CloudP2PManager();
	virtual ~CloudP2PManager();

	sigslot::signal2<CloudP2PManager*,std::shared_ptr<Channel>> SignalConnectAccepted;
	sigslot::signal2<CloudP2PManager*, bool> SignalOnlineStatusChanged;

	static bool IsValidUid(std::string uid);
	static int GetUidNumber(const std::string& uid);

	void DetectNetwork(bool redoListen = true);
protected:
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void OnDestroy();
	void OnCreate();
	void OnTimer(long timerId);

	virtual void Online();
	virtual void Offline();

	ULONGLONG mTickLastCheck;

	bool mOnline=false;
	long mTimerCheck = 0;
};

}
}
}
}
