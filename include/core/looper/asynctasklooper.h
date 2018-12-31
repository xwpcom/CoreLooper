#pragma once
#include "looper/asynctask.h"
namespace Bear {
namespace Core
{

//XiongWanPing 2017.1.02
//用来辅助执行一些可短暂阻塞的任务
class CORE_EXPORT AsyncTaskLooper :public Looper
{
	SUPER(Looper)
public:
	AsyncTaskLooper();
	virtual ~AsyncTaskLooper();
	int AddTask(std::shared_ptr<AsyncTask> obj, bool enablePreExecute = true, bool enablePostExecute = true);

protected:
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	struct tagAddTaskInfo
	{
		tagAddTaskInfo()
		{
		}

		~tagAddTaskInfo()
		{
		}

		std::shared_ptr<AsyncTask> mTask;
		bool mEnablePreExecute = false;
		bool mEnablePostExecute = false;

		std::shared_ptr<tagAddTaskInfo> mSelf;//响应后要清除
	};
	void DoTask(std::shared_ptr<tagAddTaskInfo> task);
	void OnExecuteTask(std::shared_ptr<AsyncTask> task);


	std::list<std::shared_ptr<tagAddTaskInfo>> mTasks;
	bool mBusying = false;
};
}
}