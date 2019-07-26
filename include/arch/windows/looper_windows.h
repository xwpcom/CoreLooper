#pragma once
#include "looper/handler.h"
#include "looper/looperimpl.h"
namespace Bear {
namespace Core
{
#ifdef _MSC_VER

//XiongWanPing 2016.03.xx
//windows上采用iocp来实现消息循环
class CORE_EXPORT Looper_Windows :public LooperImpl
{
	friend class Handler;
	friend class SmartTlsLooperManager;
public:
	Looper_Windows();
	virtual ~Looper_Windows();

protected:
	virtual int StartHelper(bool newThread);
	virtual void _StackLooperSendMessage(tagLoopMessageInternal& loopMsg);
	bool PostQueuedCompletionStatus(HANDLE handle, DWORD bytes = 0, ULONG_PTR key = 0, LPOVERLAPPED lpOverlapped = 0);

private:
	virtual int getMessage(tagLoopMessageInternal& msg);
};

#endif
}
}