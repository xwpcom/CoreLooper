#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {

class CloudP2PManager;

//每个设备的uid和password都不同，并且一一绑定
//uid是公开的,password是保密的
//password用来防止恶意程序故意采用同一uid来干扰正常设备的运行,这点比tutk的安全
//尚云文档中APILicense用词不当，所以我们改名为password:CRCKey
struct tagCloudP2PConfig
{
	std::string				mServerParam;
	std::string				mUid;
	std::string				mPassword;
	std::weak_ptr<Handler>	mHandler;
	UINT				mAcceptMessage = BM_NULL;
};


enum eP2PState
{
	eP2PState_Error = -1,
	eP2PState_None = 0,
	eP2PState_Ready,		//p2p就绪，可以接受客户端连接
};

enum
{
	BM_CONFIG_CLOUD_P2P,
};

//XiongWanPing 2016.07.14
//尚云p2p的侦听是阻塞的，所以只能放在单独的looper中
class CLOUD_P2P_EXT_CLASS CloudP2PListener :public BlockLooper
{
	SUPER(BlockLooper)
public:
	CloudP2PListener();
	virtual ~CloudP2PListener();
	eP2PState GetState()const
	{
		return mState;
	}

	std::string GetStateDesc()const;

protected:
	void OnDestroy();
	void OnTimer(long timerId);
	void Listen();

	int OnAccept(int sid);
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	int OnConfig(const tagCloudP2PConfig& config);
	void SwitchState(eP2PState state);

protected:
	tagCloudP2PConfig	mConfig;
	eP2PState			mState;
	long mTimerListen = 0;
};

}
}
}
}
