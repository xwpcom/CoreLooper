#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {

enum
{
	BM_CLOUD_P2P_DETECT_WORK,
};

/*
XiongWanPing 2016.12.17
CloudP2PHomeworker运行在设备端，协助其他looper来调用可能短时阻塞的的任务
目前包括:
.CloudP2PApi::NetworkDetect()
.
*/
class CLOUD_P2P_EXT_CLASS CloudP2PHomeworker :public BlockLooper
{
	SUPER(BlockLooper)
public:
	CloudP2PHomeworker();
	~CloudP2PHomeworker();

protected:
	void OnTimer(long timerId);
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	void NetworkDetect();

	long mTimerDelayNetworkDetect = 0;
};

}
}
}
}
