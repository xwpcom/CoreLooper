#include "stdafx.h"
#include "timermanager.h"
#include "looperimpl.h"
#include "timerextrainfo.h"
#include "handlerinternaldata.h"
#include "profiler.h"

#define GRANULARITY 1 //1ms
#define WHEEL_BITS1 8
#define WHEEL_BITS2 6
#define WHEEL_SIZE1 (1 << WHEEL_BITS1) //256
#define WHEEL_SIZE2 (1 << WHEEL_BITS2) //64
#define WHEEL_MASK1 (WHEEL_SIZE1 - 1)
#define WHEEL_MASK2 (WHEEL_SIZE2 - 1)
#define WHEEL_NUM 5

using namespace std;

namespace Bear {
namespace Core
{

typedef struct tagWheel
{
	tagWheel(uint32_t n);
	~tagWheel();

	tagNodeLink *spokes = nullptr;
	uint32_t size = 0;
	uint32_t spokeIndex = 0;
}tagWheel;

//从如下代码改进而来
//http://www.cnblogs.com/zhanghairong/p/3757656.html
//https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/
//它本身参考了linux kernel中的timer实现
// https://www.snellman.net/blog/archive/2016-07-27-ratas-hierarchical-timer-wheel/
// 上面这个网文提到了我关注的一个问题:
// One of the perceived problems of timer wheels is that while event insertion and deletion are O(1) operations, 
// finding out the time remaining until the next event triggers is O(m+n) where m is the total number of timer wheel slots 
TimerManager::TimerManager()
{
	mBusying = false;
	mStartTick = GetCurrentMillisec();
	mWheels[0] = new tagWheel(WHEEL_SIZE1);
	for (int i = 1; i < WHEEL_NUM; ++i)
	{
		mWheels[i] = new tagWheel(WHEEL_SIZE2);
	}
}

TimerManager::~TimerManager()
{
	for (int i = 0; i < WHEEL_NUM; ++i)
	{
		if (mWheels[i])
		{
			delete mWheels[i];
			mWheels[i] = NULL;
		}
	}
}

uint64_t TimerManager::GetCurrentMillisec()
{
	return ShellTool::GetTickCount64();// *10;//加速
}

tagWheel::tagWheel(uint32_t n) : size(n), spokeIndex(0)
{
	spokes = new tagNodeLink[n];
}

tagWheel::~tagWheel()
{
	if (spokes)
	{
		delete[]spokes;
		spokes = nullptr;
	}
}

void TimerManager::DetectList()
{
	uint64_t now = GetCurrentMillisec();
	uint64_t loopnum = now > mStartTick ? (now - mStartTick) / GRANULARITY : 0;

	tagWheel *wheel = mWheels[0];
	for (uint64_t i = 0; i < loopnum; ++i)
	{
		tagNodeLink *spoke = &wheel->spokes[wheel->spokeIndex];
		tagNodeLink *link = spoke->next;
		while (link != spoke)
		{
			tagTimerNode *node = CONTAINING_RECORD(link, tagTimerNode, link);

			//link->prev->next = link->next;
			//link->next->prev = link->prev;

			//ASSERT(link->next == node->link.next);
			//link = node->link.next;
			link = link->next;

			AddToReady(node);
		}
		ASSERT(link == spoke);
		spoke->next = spoke->prev = spoke;

		if (++wheel->spokeIndex >= wheel->size)
		{
			wheel->spokeIndex = 0;
			Cascade(1);
		}
		mStartTick += GRANULARITY;
	}

	ProcessTimeOut();
}

void TimerManager::AddToReady(tagTimerNode *node)
{
	tagNodeLink *nodelink = &(node->link);
	nodelink->prev = mReadyNodes.prev;
	mReadyNodes.prev->next = nodelink;
	nodelink->next = &mReadyNodes;
	mReadyNodes.prev = nodelink;
}

void TimerManager::ProcessTimeOut()
{
	uint64_t tickNow = GetCurrentMillisec();
	while (1)
	{
		tagNodeLink *item = mReadyNodes.next;
		if (item == &mReadyNodes)
		{
			break;
		}

		//把item从list脱离
		item->next->prev = item->prev;
		item->prev->next = item->next;

		tagTimerNode *node = CONTAINING_RECORD(item, tagTimerNode, link);
		if (node->mExtraInfo)
		{
			auto obj = node->mExtraInfo->mRunnable;
			KillTimer(node->mHandler->shared_from_this(), node->mTimerId);
			if (obj)
			{
				obj->Run();
			}
		}
		else
		{
			node->mDeadTime = tickNow + node->mInterval;
			AddTimer(node->mInterval, node);

			//在OnTimer中可以删除当前timerId或其他timerId
			//如果删除的timerId恰好已加入mReadyNodes也能保证KillTimer之后不会触发此timer
			//RemoveTimer会从mReadyNodes中移除
#ifdef _CONFIG_DEBUG_LOOPER
			const string name = node->mHandler->GetName();
			const int id = node->mTimerId;
			auto tick = ShellTool::GetTickCount64();
			if (mEnableDebugInfo)
			{
				//DV("%s.timer#begin,id=%d", name.c_str(), id);
			}
#endif

#if defined _CONFIG_PROFILER
			if (Looper::CurrentLooper()->profilerEnabled())
			{
				auto obj = Looper::CurrentLooper()->profiler();
				obj->timerCallCount++;
			}
#endif
			{
				//tagHandlerInternalData保证node->mHandler是有效的
				node->mHandler->OnTimer(node->mTimerId);
			}

			//运行到此时node->mHandler可能已销毁，所以不能再访问它

#ifdef _CONFIG_DEBUG_LOOPER
			if (mEnableDebugInfo)
			{
				tick = ShellTool::GetTickCount64() - tick;
				if (tick >= 40)
				{
					DV("%s.timer#end  ,id=%d,tick=" FMT_LONGLONG " #################################", name.c_str(), id, tick);
				}
				else
				{
					//DV("%s.timer#end  ,id=%d", name.c_str(), id);
				}
			}
#endif
		}
	}

	ASSERT(mReadyNodes.next == &mReadyNodes);
	ASSERT(mReadyNodes.prev == &mReadyNodes);

	mReadyNodes.next = mReadyNodes.prev = &mReadyNodes;
}

uint32_t TimerManager::Cascade(uint32_t wheelindex)
{
	if (wheelindex < 1 || wheelindex >= WHEEL_NUM)
	{
		return 0;
	}

	tagWheel *wheel = mWheels[wheelindex];
	int casnum = 0;
	uint64_t now = GetCurrentMillisec();
	tagNodeLink *spoke = &wheel->spokes[wheel->spokeIndex++];
	tagNodeLink *link = spoke->next;
	spoke->next = spoke->prev = spoke;
	while (link != spoke)
	{
		tagTimerNode *node = CONTAINING_RECORD(link, tagTimerNode, link);
		//tagTimerNode *node = (tagTimerNode *)link;
		link = node->link.next;
		if (node->mDeadTime <= now)
		{
			AddToReady(node);
		}
		else
		{
			uint64_t milseconds = node->mDeadTime - now;
			AddTimer((uint32_t)milseconds, node);
			++casnum;
		}
	}

	if (wheel->spokeIndex >= wheel->size)
	{
		wheel->spokeIndex = 0;
		casnum += Cascade(++wheelindex);
	}

	return casnum;
}

void TimerManager::AddTimer(uint32_t milseconds, tagTimerNode *node)
{
	tagNodeLink *spoke = NULL;
	uint32_t interval = milseconds / GRANULARITY;
	uint32_t threshold1 = WHEEL_SIZE1;
	uint32_t threshold2 = 1 << (WHEEL_BITS1 + 1 * WHEEL_BITS2);
	uint32_t threshold3 = 1 << (WHEEL_BITS1 + 2 * WHEEL_BITS2);
	uint32_t threshold4 = 1 << (WHEEL_BITS1 + 3 * WHEEL_BITS2);

	int spokeIndex = -1;
	int wheelIndex = -1;
	if (interval < threshold1)
	{
		wheelIndex = 0;
		spokeIndex = (interval + mWheels[0]->spokeIndex) & WHEEL_MASK1;
	}
	else if (interval < threshold2)
	{
		wheelIndex = 1;
		spokeIndex = ((interval - threshold1 + mWheels[1]->spokeIndex * threshold1) >> WHEEL_BITS1) & WHEEL_MASK2;
	}
	else if (interval < threshold3)
	{
		wheelIndex = 2;
		spokeIndex = ((interval - threshold2 + mWheels[2]->spokeIndex * threshold2) >> (WHEEL_BITS1 + WHEEL_BITS2)) & WHEEL_MASK2;
	}
	else if (interval < threshold4)
	{
		wheelIndex = 3;
		spokeIndex = ((interval - threshold3 + mWheels[3]->spokeIndex * threshold3) >> (WHEEL_BITS1 + 2 * WHEEL_BITS2)) & WHEEL_MASK2;
	}
	else
	{
		wheelIndex = 4;
		spokeIndex = ((interval - threshold4 + mWheels[4]->spokeIndex * threshold4) >> (WHEEL_BITS1 + 3 * WHEEL_BITS2)) & WHEEL_MASK2;
	}

	ASSERT(spokeIndex != -1);
	ASSERT(wheelIndex != -1);
	//DV("wheelIndex=%d,spokeIndex=%d", wheelIndex,spokeIndex);

	spoke = mWheels[wheelIndex]->spokes + spokeIndex;

	tagNodeLink *nodelink = &(node->link);
	nodelink->prev = spoke->prev;
	spoke->prev->next = nodelink;
	nodelink->next = spoke;
	spoke->prev = nodelink;
}

void TimerManager::RemoveTimer(tagTimerNode* node)
{
	//DW("%s(timerId=%d,interval=%d)", __func__, node->mTimerId,node->mInterval);

	tagNodeLink *nodelink = &(node->link);
	if (nodelink->prev) {
		nodelink->prev->next = nodelink->next;
	}
	if (nodelink->next) {
		nodelink->next->prev = nodelink->prev;
	}
	nodelink->prev = nodelink->next = nullptr;
}

int TimerManager::SetTimer(shared_ptr<Handler>handler, long timerId, UINT interval, shared_ptr<tagTimerExtraInfo> extraInfo)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return 0;
	}

	return SetTimer(handler.get(), timerId, interval, extraInfo);
}

int TimerManager::SetTimer(Handler *handler, long timerId, UINT interval, shared_ptr<tagTimerExtraInfo> extraInfo)
{
#ifndef _MSC_VER
	//DV("%s,id=%d", __func__, timerId);
#endif

	if (!handler)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (interval == 0)
	{
		interval = 1;
	}

	//DW("SetTimer(handler=%p,timerId=%d,interval=%d)",handler.get(),timerId,interval);

	//如果此handler已经存在timerId,先删除它
	shared_ptr<tagTimerNode> node = nullptr;
	auto timerMap = handler->mInternalData->mTimerMap;
	if (!timerMap)
	{
		handler->mInternalData->mTimerMap = make_shared<std::unordered_map<long, shared_ptr<tagTimerNode>>>();
		timerMap = handler->mInternalData->mTimerMap;
	}

	uint64_t mDeadTime = GetCurrentMillisec() + interval;

	if (node)
	{
		node->mDeadTime = mDeadTime;
		node->mInterval = interval;
	}
	else
	{
		node = make_shared<tagTimerNode>(this, handler, mDeadTime, interval);
		(*timerMap)[timerId] = node;
		node->mTimerId = timerId;
		node->mHandler = handler;
	}
	node->mExtraInfo = extraInfo;

	AddTimer(interval, node.get());
	return 0;
}

void TimerManager::KillTimer(Handler *handler, long& timerId)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return;
	}

	auto timerMap = handler->mInternalData->mTimerMap;
	if (timerMap)
	{
		auto iter = timerMap->find(timerId);
		if (iter != timerMap->end())
		{
			auto node = (*iter).second;
			timerMap->erase(iter);

			if (node)
			{
				timerId = 0;
				node->mTimerManager->RemoveTimer(node.get());
			}
			else
			{
				ASSERT(FALSE);
			}
		}
	}
}

void TimerManager::KillTimer(shared_ptr<Handler>handler, long& timerId)
{
	if (!handler)
	{
		ASSERT(FALSE);
		return;
	}

	KillTimer(handler.get(), timerId);
}

void TimerManager::clearCacheTick()
{

}

int TimerManager::ProcessTimer(DWORD& cmsDelayNext, ULONGLONG ioIdleTick)
{
	if (mBusying)/* 禁止重入 */
	{
		cmsDelayNext = 1;
		return 0;
	}

	mBusying = true;
	DetectList();
	mBusying = false;

	if (ioIdleTick < 1000)
	{
		cmsDelayNext = 1;
	}
	else
	{
		cmsDelayNext = (DWORD)GetMinIdleTime();
	}
	return 0;
}

//返回距离下一timer触发的时间，用来决定Looper能等待多久
//为简单起见,只在最低几级wheel检查,这段代码比较丑陋，至少是可以工作的了，后面再改进
long TimerManager::GetMinIdleTime()const
{
	long rewindTime0 = -1;
	long ms0 = 0;
	bool hasTimer0 = false;
	{
		const tagWheel *wheel = mWheels[0];
		const uint32_t spokeIndex = wheel->spokeIndex;
		for (uint32_t i = 0; i < wheel->size; ++i)
		{
			auto idx = (i + spokeIndex) % wheel->size;
			auto spoke = &wheel->spokes[idx];
			if (spoke->next != spoke)
			{
				hasTimer0 = true;
				break;
			}

			ms0 += GRANULARITY;
			if (idx == wheel->size - 1)
			{
				rewindTime0 = ms0;
			}
		}

		if (rewindTime0 == -1)
		{
			//还没到回绕就有timer,不需要检查上一级的wheel了
			return ms0;
		}
	}

	//wheel[0]为256ms,每个spoke为1ms
	//wheel[1]为256ms*64,每个spoke为256ms

	//在wheel[1].spokeIndex中查找,
	{
		auto wheel = mWheels[1];
		auto spoke = &wheel->spokes[wheel->spokeIndex];
		if (spoke->next != spoke)
		{
			//由于一个spoke中可能有超多timer,并且各timer是没有排序的,不适合遍历整个spoke来获取最小tick的timer
			//所以假定wheel[0]需要Cascade时即触发下一timer
			return rewindTime0;
		}

		if (!hasTimer0)
		{
			long rewindTime1 = -1;
			long ms1 = 0;
			bool hasTimer1 = false;

			{
				const tagWheel *wheel = mWheels[1];
				const uint32_t spokeIndex = wheel->spokeIndex;
				for (uint32_t i = 0; i < wheel->size; ++i)
				{
					auto idx = (i + spokeIndex) % wheel->size;
					auto spoke = &wheel->spokes[idx];
					if (spoke->next != spoke)
					{
						hasTimer1 = true;
						break;
					}

					ms1 += 256;
					if (idx == wheel->size - 1)
					{
						rewindTime1 = ms1;
					}
				}

				if (rewindTime1 == -1)
				{
					//还没到回绕就有timer,不需要检查上一级的wheel了
					return ms1;
				}

				{
					auto wheel = mWheels[2];
					auto spoke = &wheel->spokes[wheel->spokeIndex];
					if (spoke->next != spoke)
					{
						//由于一个spoke中可能有超多timer,并且各timer是没有排序的,不适合遍历整个spoke来获取最小tick的timer
						//所以假定wheel[0]需要Cascade时即触发下一timer
						return rewindTime1;
					}

					if (!hasTimer1)
					{
						long rewindTime2 = -1;
						long ms2 = 0;
						bool hasTimer2 = false;

						//继续在wheel[2].spokeIndex中查找
						//由于wheel[1]已能达到16384ms睡眠，已经足够了，所以不再继续
						{
							const tagWheel *wheel = mWheels[2];
							const uint32_t spokeIndex = wheel->spokeIndex;
							for (uint32_t i = 0; i < wheel->size; ++i)
							{
								auto idx = (i + spokeIndex) % wheel->size;
								auto spoke = &wheel->spokes[idx];
								if (spoke->next != spoke)
								{
									hasTimer2 = true;
									break;
								}

								ms2 += 256*64;
								if (idx == wheel->size - 1)
								{
									rewindTime2 = ms2;
								}
							}

							if (rewindTime2 == -1)
							{
								//还没到回绕就有timer,不需要检查上一级的wheel了
								return ms2;
							}

							//目前已达256*64*64=1048576 ms
							//可继续检查wheel3

							return ms2;
						}

					}
				}

				return ms1;
			}
		}
	}

	return ms0;
}

}
}
