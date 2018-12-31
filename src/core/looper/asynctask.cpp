#include "stdafx.h"
#include "asynctask.h"
#include "asynctasklooper.h"

using namespace std;
using namespace Bear::Core;

int AsyncTask::Execute(bool enablePreExecute, bool enablePostExecute)
{
	auto mainLooper = Looper::GetMainLooper();
	if (!mainLooper || !Looper::CurrentLooper())
	{
		return -1;
	}

	if (!IsCreated())
	{
		mIsAutoCreated = true;
		Create(Looper::CurrentLooper()->shared_from_this());
	}

	//这里有竞争，可能导致创建多个AsyncTaskLooper
	//发生的几率很小，并且即使有多个AsyncTaskLooper则不影响正常的使用,所以没做加锁处理
	auto looper = _MObject(AsyncTaskLooper,"AsyncTaskLooper");
	if(!looper)
	{
		looper = make_shared<AsyncTaskLooper>();
		mainLooper->AddChild(looper);
		looper->Start();
	}

	if (looper)
	{
		looper->AddTask(dynamic_pointer_cast<AsyncTask>(shared_from_this()), enablePreExecute, enablePostExecute);
		return 0;
	}

	return -1;
}
