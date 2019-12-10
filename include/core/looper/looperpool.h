#pragma once

namespace Bear {
namespace Core
{

//XiongWanPing 2019.04.21
class LooperPool:public Handler
{
	SUPER(Handler)
public:
	LooperPool();

	void LOOPER_SAFE SetConfig(int maxLoopers,int minLoopers=0);
protected:
	void OnCreate();
	void OnTimer(long id);

	struct tagConfig
	{
		int mMaxLoopers=0;//default: no limit
		int mMinLoopers=0;//default:destroy all work looper when idle for a while
	}mConfig;

	weak_ptr<Looper> mWorkLoopers;
};

}
}
