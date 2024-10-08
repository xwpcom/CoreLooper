﻿#include "stdafx.h"
#include "handler.h"
#include "timernode.h"
#include "timermanager.h"
#include "handlerinternaldata.h"
#include "looperinternaldata.h"
#include "message.inl"

using namespace std;

namespace Bear {
namespace Core
{
#define TAG "handlerData"

#ifdef _MSC_VER_DEBUG
//#define _TEST_TIMER_ID_REWIND	//测试timerId回绕
#endif

#ifdef _CONFIG_MONITOR_HANDLER
CriticalSection tagHandlerInternalData::gCSBaseHandler;
std::unordered_map<long*, long*> tagHandlerInternalData::gHandlers;
int tagHandlerInternalData::gRationalHandlerUpperLimit = 0;// 1000;
#endif

static CriticalSection gInternalDataCS;
//目前仅在如下场景用到:AddChildHelper，用来避免多个looper同时对同一handler调用AddChild引起竞争
//Get/SetObjectName,保证跨looper访问mObjectName的安全
//AddChild/GetObjectName/SetObjectName都不会被非常频繁的调用，所以不太可能是瓶颈

tagHandlerInternalData::tagHandlerInternalData(Handler* handler)
{
	//要等到Create()时才和looper绑定

	mHandler = handler;
	mPassive = true;
	mCreated = false;
	mOnCreateCalled = false;
	mDestroy = false;
	mDestroyMarkCalled = false;
	mOnDestroyCalled = false;
	mIsLooper = false;
	mMaybeLongBlock = false;
	mTimerIdRewind = false;

	//LogV(TAG,"%s,this=%p", __func__, this);
#ifdef _TEST_TIMER_ID_REWIND
	mNextTimerId = -2;//for test
#endif

	//LogV(TAG,"sizeof(tagHandlerInternalData)=%d", sizeof(tagHandlerInternalData));//192
}

//corelooper框架保证tagHandlerInternalData总是在handler的原生looper中析构(几乎是100%)

tagHandlerInternalData::~tagHandlerInternalData()
{
	//LogV(TAG,"%s,this=%p", __func__, this);

	if (!mIsLooper)
	{
		RemoveAllTimer();
	}

	if (mParent)
	{
		mParent->mInternalData->RemoveChildWeakRef(mHandler);
	}
}

//规定0是无效timerId
//其他数值都是有效的,包括负数
long tagHandlerInternalData::NextTimerId()
{
	//ASSERT(IsMyselfThread());

	if (!mTimerMap || mTimerMap->size() == 0)
	{
#ifdef _TEST_TIMER_ID_REWIND
		if (!mTimerMap)
		{
			mTimerMap = make_shared<std::map<long, shared_ptr<tagTimerNode>>>();
		}
#else
		mTimerIdRewind = false;
		mNextTimerId = 0;
		return ++mNextTimerId;
#endif
	}

	//检查是否能复用timer id
	if (mNextTimerId != 0)
	{
		auto iter = mTimerMap->find(mNextTimerId);
		if (iter == mTimerMap->end())
		{
			return mNextTimerId;
		}
	}

	auto id = ++mNextTimerId;
	if (id == 0)
	{
		mTimerIdRewind = true;
		++mNextTimerId;
	}

	//考虑mNextTimerId回绕的情况
	if (!mTimerIdRewind)
	{
		return id;
	}

	while (1)
	{
		auto iter = mTimerMap->find(mNextTimerId);
		if (iter == mTimerMap->end())
		{
			return mNextTimerId;
		}

		++mNextTimerId;
	}

	ASSERT(FALSE);
	return -1;
}

void tagHandlerInternalData::RemoveAllTimer()
{
	auto& timerMap = mTimerMap;
	if (timerMap)
	{
		for (auto iter = timerMap->begin(); iter != timerMap->end();)
		{
			auto info = (*iter).second;
			int id = (int)info->mTimerId;
			//timerId<0为内部特殊timer,不要清除，目前timerId=-1表示looper定时检测是否能退出timer
			if (id >= 0)
			{
				info->mTimerManager->RemoveTimer(info.get());

				auto iterSave = iter++;
				timerMap->erase(iterSave);
			}
			else
			{
				++iter;
			}
			//info->mSlot->RemoveNode(&info->mNode);
		}

		if (timerMap->size() == 0)
		{
			timerMap = nullptr;
		}
	}
}

long tagHandlerInternalData::GetLiveChildrenCount()
{
	int count = 0;

	for (auto iter = mChildren.begin(); iter != mChildren.end();)
	{
		auto child = iter->second.lock();
		if (child)
		{
			++iter;
			++count;
		}
		else
		{
			auto iterSave = iter;
			++iter;
			mChildren.erase(iterSave);
		}
	}

	return count;
}

int tagHandlerInternalData::AddChildHelper(weak_ptr<Handler> wpChild, string name)
{
	ASSERT(mHandler->IsMyselfThread());

	auto child = wpChild.lock();
	if (!child)
	{
		return -1;
	}

//#define _CONFIG_TEST_CONTEST	//测试竞争

#ifdef _CONFIG_TEST_CONTEST
	srand((int)ShellTool::GetTickCount64());
#endif

	bool callCreate = false;
	bool callDestroy = false;

	{
		//要考虑各种极端情况,
		//比如用户故意把handler添加到不同的looper中,下面的代码会出现竞争,所以要做同步

		AutoLock lock(&gInternalDataCS);

		if (child->mInternalData->mParent || (!child->IsLooper() && child->IsCreated()))
		{
			if (child->mInternalData->mParent)
			{
				LogW(TAG,"child->mInternalData->mParent is NOT null");
			}

			if (child->IsCreated())
			{
				LogW(TAG,"child is already created");
			}

			LogW(TAG,"%s fail", __func__);
			//ASSERT(FALSE);
			return -1;
		}

		if (!mHandler->IsLooper() && !child->IsLooper())
		{
			//子handler必须和parent handler位于同一looper
			/*
			if (mHandler->GetThreadId() != child->GetThreadId())
			{
				LogW(TAG,"%s fail,handler must be in the same looper as parent handler", __func__);
				return -1;
			}
			*/
		}

		if (!name.empty())
		{
			child->SetObjectName(name);
		}

		auto iter = mChildren.find((long*)child.get());
		if (iter != mChildren.end())
		{
			//object析构后可能在同一内存重新创建此object,所以可能运行到此
			//不影响使用
			//ASSERT(FALSE);
			//return -1;
		}

		if (mHandler->IsLooper())
		{
			//子looper挂到父looper时，如果没有设定exit event,则自动设定owner looper
			auto parentLooper = dynamic_pointer_cast<Looper>(mHandler->shared_from_this());
			if (parentLooper)
			{
				auto looper = dynamic_pointer_cast<Looper>(child);
				if (looper && !looper->GetExitEvent())
				{
					if (!looper->GetOwnerLooper())
					{
						looper->SetOwnerLooper(parentLooper);
					}
				}
			}

		}

		if (!child->IsLooper())
		{
			//考虑跨looper创建普通handler
			auto parentLooper = dynamic_pointer_cast<Looper>(Looper::CurrentLooper()->shared_from_this());
			child->mThreadId = mHandler->mThreadId;
			child->mInternalData->mLooper = parentLooper;

			if (!parentLooper)
			{
				LogW(TAG,"parentLooper is null");
			}
		}

		child->mInternalData->mParent = mHandler->shared_from_this();
		mChildren[(long*)child.get()] = wpChild;

		if (child->mInternalData->mPassive)
		{
			child->mInternalData->mSelfRef = child->shared_from_this();//被动对象引用自己来保活,will be clear on BM_DESTROY
		}
		
		//适时触发初始化

		if (!child->mInternalData->mCreated && !child->IsLooper())
		{
			callCreate = true;
		}

		//parent已销毁时，马上销毁child
		if (mDestroy)
		{
			callDestroy = true;
		}
	}

	auto ret = -1;
	//注意:callCreate和callDestroy同时为true时不要抵消，要按正常流程走完

	if (callCreate)
	{
		ret = (int)child->sendMessage(BM_CREATE);
	}
	
	if (callDestroy)
	{
		child->Destroy();
	}

	return ret;
}

int tagHandlerInternalData::RegisterShortcut_Impl(const string& name, weak_ptr<Handler>obj)
{
	if (!mShortcuts)
	{
		mShortcuts = make_shared<unordered_map<string, weak_ptr<Handler>>>();
	}

	auto handler = obj.lock();
	if (handler)
	{
		(*mShortcuts)[name] = obj;
	}
	else
	{
		mShortcuts->erase(name);
	}

	return 0;
}

shared_ptr<Handler> tagHandlerInternalData::Shortcut_Impl(const std::string& name)
{
	if (mShortcuts)
	{
		auto iter = mShortcuts->find(name);
		if (iter != mShortcuts->end())
		{
			return iter->second.lock();
		}
	}

	return nullptr;
}

shared_ptr<Handler> tagHandlerInternalData::FindObject_Impl(const string& url)
{
	ASSERT(mHandler->IsMyselfThread());

	bool directChild = false;
	{
		//只缓存直接child
		auto pos = url.find('/');
		if (pos == string::npos)
		{
			directChild = true;

			if (mCacheChilds)
			{
				auto it = mCacheChilds->find(url);
				if (it != mCacheChilds->end())
				{
					auto obj=it->second.lock();
					if (obj && obj->GetObjectName() == url)
					{
						return obj;
					}
				}
			}
		}
	}

	string item;
	shared_ptr<Handler> obj;
	TextSeparator demux(url.c_str(), "/");
	int level = 0;
	while (1)
	{
		int ret = demux.GetNext(item);
		if (ret)
		{
			break;
		}
		++level;

		if (!obj)
		{
			obj = mHandler->shared_from_this();
		}
		/*
		http://127.0.0.1/proc.xml?url=IotServer[SW.00168FXP]
		*/

		auto pos1 = item.find('[');
		if (pos1 != string::npos)
		{
			auto pos2 = item.find(']',pos1);
			if (pos2 != string::npos)
			{
				auto name = item.substr(0, pos1);
				auto token = item.substr(pos1+1, pos2 - pos1-1);
				
				auto root=obj->GetChild(name, nullptr);
				if (!root)
				{
					LogV(TAG, "no find [%s]", name.c_str());
					return nullptr;
				}

				auto child = root->mapChild(token);
				if (!child)
				{
					LogV(TAG, "no find [%s]", item.c_str());
					return nullptr;
				}

				obj = child;
				continue;
			}
		}

		auto child = obj->GetChild(item, nullptr);
		if (!child)
		{
			LogV(TAG,"no find [%s]", item.c_str());
			return nullptr;
		}

		obj = child;
	}

	if (directChild && obj)
	{
		if (!mCacheChilds)
		{
			mCacheChilds = make_shared<unordered_map<string, weak_ptr<Handler>>>();
		}

		(*mCacheChilds)[url] = obj;
	}

	return obj;
}

shared_ptr<Handler> tagHandlerInternalData::GetChild_Impl(LONG_PTR id)
{
	//ASSERT(id);
	ASSERT(mHandler->IsMyselfThread());

	for (auto& iter:mChildren)
	{
		auto item = iter.second.lock();
		if (item && item->GetId() == id)
		{
			return item;
		}
	}

	return nullptr;
}

long tagHandlerInternalData::GetChildCount()const
{
	return (long)mChildren.size();
}

void tagHandlerInternalData::RemoveChildWeakRef(Handler *handler)
{
	if (mHandler->IsMyselfThread())
	{
		RemoveChildWeakRef_Impl(handler);
	}
	else
	{
		mHandler->sendMessage(BM_REMOVE_CHILD_WEAKREF, (WPARAM)handler);
	}
}

void tagHandlerInternalData::RemoveChildWeakRef_Impl(Handler *handler)
{
	ASSERT(mHandler->IsMyselfThread());

	if (!handler)
	{
		return;
	}

	auto iter = mChildren.find((long*)handler);
	if (iter != mChildren.end())
	{
		mChildren.erase(iter);

		//OnChildDetach();
	}
	else
	{
		ASSERT(FALSE);
	}
}

//注意:SetTimer/SetTimerEx/KillTimer在Handler和Looper的实现是不同的，所以这里要区分

long tagHandlerInternalData::SetTimerEx(UINT interval, shared_ptr<tagTimerExtraInfo> info)
{
	if (mHandler->IsLooper())
	{
		auto obj = (LooperImpl *)(mHandler);
		return obj->SetTimerEx(mHandler, interval, info);
	}

	if (mLooper)
	{
		if (!mTimerManager)
		{
			auto obj = dynamic_cast<LooperImpl*>(mLooper.get());
			mTimerManager = obj->GetTimerManager();
		}

		return mLooper->SetTimerEx(mHandler, interval, info);
	}

	LogW(TAG,"Fail %s due to mLooper is null", __func__);
	return 0;
}

long tagHandlerInternalData::SetTimer(UINT interval)
{
	if (mHandler->IsLooper())
	{
		auto obj = (LooperImpl *)(mHandler);
		return obj->SetTimer(mHandler, interval);
	}

	if (mLooper)
	{
		if (!mTimerManager)
		{
			auto obj = dynamic_cast<LooperImpl*>(mLooper.get());
			mTimerManager = obj->GetTimerManager();
		}

		return mLooper->SetTimer(mHandler, interval);
	}

	LogW(TAG,"Fail %s due to mLooper is null", __func__);
	return 0;
}

void tagHandlerInternalData::KillTimer(long& timerId)
{
	if (mHandler->IsLooper())
	{
		auto obj = (LooperImpl *)(mHandler);
		obj->KillTimer(mHandler, timerId);
		return;
	}

	if (!mLooper)
	{
		return;
	}

	mLooper->KillTimer(mHandler, timerId);
}

string tagHandlerInternalData::GetName()const
{
	string name;
	name.reserve(64);//should be enough for most case
	{
		AutoLock lock(&gInternalDataCS);
		name = mObjectName;
	}

	return name;
}

void tagHandlerInternalData::SetName(const string& name)
{
	auto str = name;
	
	//std move speed up
	AutoLock lock(&gInternalDataCS);
	mObjectName= std::move(str);
}

void tagHandlerInternalData::Dump(int level, bool includingChild)
{
	//#ifdef _DEBUG
	string indent;
	for (int i = 0; i < level; ++i)
	{
		indent += StringTool::Format("    ");
	}

	if (includingChild && mChildren.size()>0)
	{
		LogV(TAG,"%s%s#begin", indent.c_str(), mHandler->GetObjectName().c_str());
		{
			for (auto iter = mChildren.begin(); iter != mChildren.end();)
			{
				auto child = iter->second.lock();
				if (child)
				{
					if (child->IsLooper())
					{
						if (!child->MaybeLongBlock())
						{
							auto looper = dynamic_pointer_cast<Looper>(child);
							//LogV(TAG,"this=0x%08x,looper=0x%08x(%s)", this,looper.get(),looper->mObjectName.c_str());
							looper->postMessage(BM_DUMP, (WPARAM)(long)(level + 1),(LPARAM)includingChild);//这里用sendMessage可能导致死锁,见2016.12.13文档//
						}
					}
					else
					{
						child->mInternalData->Dump(level + 1);
					}

					++iter;
				}
				else
				{
					auto iterSave = iter;
					++iter;
					mChildren.erase(iterSave);
				}
			}
		}
		LogV(TAG,"%s%s#end", indent.c_str(), mHandler->GetObjectName().c_str());
	}
	else
	{
		string extraInfo;
		if (mIsLooper)
		{
			auto looper = dynamic_pointer_cast<LooperImpl>(mHandler->shared_from_this());
			if (looper)
			{
				if (looper->mLooperInternalData->mAttachThread)
				{
					extraInfo += ",AttachThread.Looper";
				}
			}
		}

		LogV(TAG,"%s%s,this=0x%08x,url=[%s]%s", indent.c_str(), mHandler->GetObjectName().c_str(), this, mHandler->GetUrl().c_str(), extraInfo.c_str());
	}
	//#endif
}

#ifdef _CONFIG_MONITOR_HANDLER
void tagHandlerInternalData::DumpAll()
{
	AutoLock lock(&gCSBaseHandler);
	if (gHandlers.empty())
	{
		LogV(TAG,"%s# is empty", __func__);
	}
	else
	{
		LogW(TAG,"%s#begin,", __func__);
		for (auto& iter :gHandlers)
		{
			Handler *obj = (Handler *)iter.second;
			if (obj)
			{
				bool includingChild = false;
				obj->mInternalData->Dump(0,includingChild);
			}
		}
		LogW(TAG, "%s#end", __func__);
	}
}

struct tagItem
{
	string name;
	int count = 0;
	ULONGLONG bytes = 0;
};

static bool
handler_compareCount(tagItem& item1, tagItem& item2)
{
	return item1.count > item2.count;
}
static bool
handler_compareBytes(tagItem& item1, tagItem& item2)
{
	return item1.bytes > item2.bytes;
}

int tagHandlerInternalData::fetchHandlerInfo(JsonObject& json)
{
#ifdef _MSC_VER
	AutoLock lock(&gCSBaseHandler);

	map<string, tagItem> handlerCounts;//统计相同name的个数
	for (auto& iter : gHandlers)
	{
		Handler* obj = (Handler*)iter.second;
		if (obj)
		{
			auto bytes = obj->memoryUsed();

			auto name = obj->GetObjectName();
			auto& item = handlerCounts[name];
			item.name = name;
			item.count++;
			item.bytes += bytes;
		}
	}

	vector <tagItem> items;
	items.reserve(handlerCounts.size());
	for (auto& item : handlerCounts)
	{
		items.emplace_back(tagItem{ item.first,item.second.count,item.second.bytes });
	}
	
	{
		sort(items.begin(), items.end(), handler_compareCount);

		auto& jItems = json.createNestedArray("itemCounts");
		for (auto& item : items)
		{
			auto& jItem = jItems.createNestedObject();
			jItem["name"] = item.name;
			jItem["count"] = item.count;
		}
	}
	{
		sort(items.begin(), items.end(), handler_compareBytes);

		auto& jItems = json.createNestedArray("itemBytes");
		for (auto& item : items)
		{
			auto& jItem = jItems.createNestedObject();
			jItem["name"] = item.name;
			jItem["bytes"] = item.bytes;
		}
	}
#endif

	return 0;
}

//返回handler总数
//如果count持续上升，超过合理范围，说明有handler泄漏
int tagHandlerInternalData::GetHandlerCount()
{
	AutoLock lock(&gCSBaseHandler);
	return (int)gHandlers.size();
}

void tagHandlerInternalData::SetRationalHandlerUpperLimit(int count)
{
	gRationalHandlerUpperLimit = count;
}
#endif

void tagHandlerInternalData::TestContestSleep()
{
#ifdef _CONFIG_TEST_CONTEST
	ShellTool::Sleep(rand() % 100);//故意测试竞争
#endif
}

//gc时如果没能成功析构Handler,可能多次调用本接口
//可用来检测不能及时析构的Handler
void tagHandlerInternalData::OnPrepareDestructor()
{
	auto tickNow = ShellTool::GetTickCount64();

	auto seconds = (tickNow - mTickDestroy) / 1000;
	if (seconds >= 10)
	{
		LogW(TAG,"%s is still alive for %lld seconds after called Destroy()",GetName().c_str(), seconds);
	}
}

}
}
