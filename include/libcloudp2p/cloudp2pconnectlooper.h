#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {
//XiongWanPing 2016.12.27
//为加速p2p连接,CloudP2PConnecter每次创建两个CloudP2PConnectLooper
//一个启用lan search和p2p,在ipcam只用3g,不用wifi时，要10秒才能连接成功
//另一个只使用server relay,在ipcam只用3g,不用wifi时，一般能在500ms连接成功
//CloudP2PConnecter会采用连接速度最快的
class CloudP2PConnectLooper :public BlockLooper
{
	SUPER(BlockLooper);
public:
	CloudP2PConnectLooper();
	~CloudP2PConnectLooper();
	int SetConnectInfo(std::weak_ptr<Looper>looper, UINT msg, std::string uid, BYTE flags);
protected:
	void OnCreate();

	std::weak_ptr<Looper> mConnectorlooper;
	UINT	mMsg=0;//post to mConnectorlooper

	std::string	mUid;
	BYTE	mConnectFlags=0;
};
}
}
}
}