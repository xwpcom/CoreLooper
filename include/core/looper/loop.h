#pragma once

#include "handlerex.h"

namespace Bear {
namespace Core
{
class Handler;
//tagLoopMessage供外界调用时使用
struct tagLoopMessage
{
	tagLoopMessage()
	{
		mMsg = 0;
		mWP = 0;
		mLP = 0;
		mHandler = nullptr;
	}

	std::shared_ptr<Handler> mHandler;
	UINT	mMsg;
	WPARAM	mWP;
	LPARAM	mLP;
};

#include "base/object.h"
#include "looper/handler.h"
//XiongWanPing 2016.01.08
//Loop抽象类提供外界调用的接口
class CORE_EXPORT Loop :public HandlerEx
{
	friend class  Handler;
	friend struct tagHandlerInternalData;
public:
	virtual ~Loop() {}

	virtual int Start() = 0;
	virtual int StartRun() = 0;

	virtual int PostQuitMessage(long exitCode = 0) = 0;
	virtual int GetQuitCode()const = 0;

	virtual LRESULT sendMessage(std::shared_ptr<Handler> handler, UINT msg, WPARAM wp = NULL, LPARAM lp = NULL) = 0;
	virtual LRESULT postMessage(std::shared_ptr<Handler> handler, UINT msg, WPARAM wp = NULL, LPARAM lp = NULL) = 0;
	virtual LRESULT sendMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL) = 0;
	virtual LRESULT postMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL) = 0;
	virtual bool IsRunning()const = 0;

	virtual LPVOID GetLooperHandle()const = 0;

	virtual int Wakeup() = 0;
protected:
	virtual bool CanQuitLooperNow() = 0;
	virtual long SetTimer(Handler *handler, UINT interval) = 0;
	virtual long SetTimer(std::shared_ptr<Handler> handler, UINT interval) = 0;
	virtual void KillTimer(std::shared_ptr<Handler>handler, long& timerId) = 0;
	virtual void KillTimer(Handler *handler, long& timerId) = 0;
private:
	virtual long SetTimerEx(std::shared_ptr<Handler>handler, UINT interval, std::shared_ptr<tagTimerExtraInfo> extraInfo) = 0;
	virtual long SetTimerEx(Handler *handler, UINT interval, std::shared_ptr<tagTimerExtraInfo> extraInfo) = 0;
};
}
}
