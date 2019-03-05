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

	auto looper = _MObject(AsyncTaskLooper, "AsyncTaskLooper");
	if(!looper)
	{
		//在mainlooper中创建
		auto func = [=, &looper]()
		{
			ASSERT(mainLooper->IsCurrentThread());

			//find again
			looper = _MObject(AsyncTaskLooper, "AsyncTaskLooper");
			if (!looper)
			{
				looper = make_shared<AsyncTaskLooper>();
				mainLooper->AddChild(looper);
				looper->Start();
			}
		};

		mainLooper->sendRunnable(std::bind(func));
	}

	if (looper)
	{
		looper->AddTask(dynamic_pointer_cast<AsyncTask>(shared_from_this()), enablePreExecute, enablePostExecute);
		return 0;
	}

	return -1;
}
