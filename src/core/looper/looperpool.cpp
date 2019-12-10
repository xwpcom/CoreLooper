#include "stdafx.h"
#include "looperpool.h"

namespace Bear {
namespace Core
{

LooperPool::LooperPool()
{

}

void LooperPool::OnCreate()
{
	__super::OnCreate();
}

void LooperPool::OnTimer(long id)
{
	__super::OnTimer(id);
}

void LooperPool::SetConfig(int maxLoopers, int minLoopers)
{
	auto fn = [&]()
	{
		mConfig.mMaxLoopers = maxLoopers;
		mConfig.mMinLoopers = minLoopers;
	};

	if (IsMyselfThread() || !IsCreated())
	{
		fn();
	}
	else
	{
		sendRunnable(std::bind(fn));
	}
}

}
}
