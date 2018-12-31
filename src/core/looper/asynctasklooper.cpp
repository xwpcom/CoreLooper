#include "stdafx.h"
#include "asynctasklooper.h"
#include "message.inl"

using namespace std;

namespace Bear {
namespace Core
{

enum
{
	BM_ADD_TASK,
};

AsyncTaskLooper::AsyncTaskLooper()
{
	SetObjectName("AsyncTaskLooper");
	mThreadName = GetObjectName();
	//DV("%s,this=%p",__func__,this);
}

AsyncTaskLooper::~AsyncTaskLooper()
{
}

void AsyncTaskLooper::DoTask(shared_ptr<tagAddTaskInfo> info)
{
	mBusying = true;
	auto task = info->mTask;
	//DV("AsyncTaskLooper#begin task:%s", task->GetObjectName().c_str());

	if (info->mEnablePreExecute)
	{
		task->sendMessage(BM_PRE_EXECUTE);
	}

	OnExecuteTask(info->mTask);

	if (info->mEnablePostExecute)
	{
		task->sendMessage(BM_POST_EXECUTE);
	}

	if (task->IsAutoCreated())
	{
		task->Destroy();
	}

	//DV("AsyncTaskLooper#end task:%s", task->GetObjectName().c_str());
	info->mSelf = nullptr;
	mBusying = false;
}

LRESULT AsyncTaskLooper::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_ADD_TASK:
	{
		//DoTask中sendMessage时可能导致AsyncTaskLooper再次收到BM_ADD_TASK,从而导致多个task嵌套执行
		//所以采用mBusying+缓存来保证串行执行
		tagAddTaskInfo *info = (tagAddTaskInfo*)wp;
		if (mBusying)
		{
			//当前忙，加入缓存
			mTasks.push_back(info->mSelf);

			if (mTasks.size() >= 3)
			{
				DV("mTasks.size=%d", mTasks.size());
			}
			return 0;
		}

		DoTask(info->mSelf);

		ASSERT(!mBusying);

		while (!mBusying)
		{
			if (mTasks.empty())
			{
				break;
			}

			auto info = mTasks.front();
			mTasks.pop_front();
			DoTask(info);
		}

		return 0;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

int AsyncTaskLooper::AddTask(shared_ptr<AsyncTask> obj, bool enablePreExecute, bool enablePostExecute)
{
	auto info = make_shared<tagAddTaskInfo>();
	info->mTask = obj;
	info->mSelf = info;
	info->mEnablePreExecute = enablePreExecute;
	info->mEnablePostExecute = enablePostExecute;
	auto ret = postMessage(BM_ADD_TASK, (WPARAM)info.get());
	if (ret == -1)
	{
		info->mSelf = nullptr;
		return -1;
	}

	return 0;
}

void AsyncTaskLooper::OnExecuteTask(shared_ptr<AsyncTask> task)
{
	if (task)
	{
		task->Run();
	}
}
}

}
