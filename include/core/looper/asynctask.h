#pragma once

//XiongWanPing 2017.11.02
/*
AsyncTask仿照android的AsyncTask,用来在工作线程中执行一段代码
{
	auto obj = make_shared<WifiScanner>();
	obj->Execute();

	执行完后可自动销毁obj,实现和android下AsyncTask一样的效果
}
*/

namespace Bear {
namespace Core
{
class CORE_EXPORT AsyncTask :public Handler
{
	friend class Handler;
	friend class AsyncTaskLooper;

public:
	virtual ~AsyncTask() {}

	//.在looper环境下调用
	//.必须在设定了MainLooper的情况下才能调用
	virtual int Execute(bool enablePreExecute = true, bool enablePostExecute = true);

protected:
	//在AsyncTask所在looper被调用
	virtual void OnPreExecute() {};

	//在AsyncTask所在looper被调用
	virtual void OnPostExecute() {};

	//在AsyncTaskLooper中被调用
	virtual void Run() = 0;

	//OnPreExecute,Run和OnPostExecute依次串行执行，不需要做同步处理

	bool IsAutoCreated()const
	{
		return mIsAutoCreated;
	}

	bool mIsAutoCreated = false;//自动创建时，在执行完后由AsyncTaskLooper自动销毁
};
}
}
