#include "stdafx.h"
#include "looper_windows.h"
#include "thread/autolock.h"
#include "iocpobject.h"
#include "iocontext.h"
#include "looper/handler.h"
#include "looper/looper.h"
#include "../../core/looper/handlerinternaldata.h"
#include "../../core/looper/looperinternaldata.h"
#pragma comment(lib, "WS2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Bear::Core::Net;

namespace Bear {
namespace Core
{
Looper_Windows::Looper_Windows()
{
	SetObjectName("Looper_Windows");
	mThreadName = GetObjectName();
	//DV("%s,this=0x%08x", __func__, this);

	{
		static bool first = true;
		if (first)
		{
			first = false;

			SockTool::InitSockTool();//现在还有不用网络的app吗?
		}
	}
}

Looper_Windows::~Looper_Windows()
{
	//DV("%s,this=0x%08x", __func__, this);
}

int Looper_Windows::StartHelper(bool newThread)
{
	if (mLooperInternalData->mLooperRunning)
	{
		DW("mLooperRunning=%d,CurrentLooper=%p", mLooperInternalData->mLooperRunning, CurrentLooper());
		ASSERT(FALSE);
		return 0;
	}

	{
		if (!mLooperInternalData->mExitEvent)
		{
			auto owner = mLooperInternalData->mOwnerLooper.lock();
			if (owner)
			{
				SetExitEvent(owner->CreateExitEvent());
			}
		}
	}

	//ASSERT(mExitEvent);

	{
		ASSERT(mLooperHandle == INVALID_HANDLE_VALUE);
		mLooperHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	}

	mInternalData->mSelfRef = shared_from_this();

	mLooperInternalData->mLooperRunning = true;
	mLooperInternalData->mBMQuit = false;

	if (newThread)
	{
		mInternalData->mCreated = true;
		BOOL ok = ShellTool::QueueUserWorkItem((LPTHREAD_START_ROUTINE)_WorkThreadCB, (LPVOID)this, 0);
		return ok ? 0 : -1;
	}

	LooperImpl *looperCache = CurrentLooper();
	mInternalData->mCreated = true;
	_WorkThreadCB(this);
	SetCurrentLooper(looperCache);
	return 0;
}

void Looper_Windows::_StackLooperSendMessage(tagLoopMessageInternal& loopMsg)
{
	//构造一个stack looper来实现sendMessage
	auto looper = make_shared<Looper_Windows >();
	SetCurrentLooper(looper.get());
	looper->mThreadId = ShellTool::GetCurrentThreadId();
	looper->mLooperInternalData->mAttachThread = true;
	looper->mLooperHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	sendMessageHelper(loopMsg, *looper.get());
	SetCurrentLooper(nullptr);
	looper = nullptr;
}

int Looper_Windows::getMessage(tagLoopMessageInternal& msg)
{
	do
	{
		{
			AutoLock lock(&mLooperInternalData->mMessageLock);
			auto& msgList = mLooperInternalData->mMessageList;
			if (!msgList.empty())
			{
				msg = msgList.front();
				msgList.pop_front();
				return 0;
			}
		}

		if (mLooperInternalData->mBMQuit && CanQuitLooperNow())
		{
			return -1;
		}

		DWORD cmsDelayNext = INFINITE;
		bool needWait = (ProcessTimer(cmsDelayNext) != 0);
		//cmsDelayNext = MAX(cmsDelayNext, 5);
		cmsDelayNext = MAX(1, cmsDelayNext);

		//if (needWait)
		{
			//ASSERT(cmsDelayNext);//如果为0，会形成busy loop,占用大量cpu
			//DV("%s#delay=%d ms", mThreadName.c_str(),cmsDelayNext);

			BOOL ret = FALSE;

			DWORD bytes = 0;
			ULONG_PTR ptr = NULL;
			LPOVERLAPPED ov = NULL;
			ret = GetQueuedCompletionStatus(mLooperHandle, &bytes, &ptr, &ov, cmsDelayNext);

			if (ret == 0)
			{
				int error = GetLastError();
				if (error != WAIT_TIMEOUT)
				{
					if (error != 995)
					{
						//DV("fail,GetQueuedCompletionStatus error=%d", error);
					}

					if (ov == NULL && error == ERROR_ABANDONED_WAIT_0)
					{
						int x = 0;
					}
					else
					{
						int x = 0;
					}
				}
			}

			if (ptr > 0xFFFF)
			{
				ASSERT(ov);

				IocpObject *obj = (IocpObject *)ptr;
				IoContext *context = CONTAINING_RECORD(ov, IoContext, mOV);
				obj->DispatchIoContext(context, bytes);
			}
			else if (ptr && ptr <= 0xFFFF)
			{
				UINT msg = (UINT)ptr;
				LPVOID info = (LPVOID)(ULONGLONG)bytes;
				LPVOID obj = (LPVOID)ov;

				if (obj > (LPVOID)0xFFFF)
				{
					IocpObject *iocpObject = (IocpObject *)obj;
					iocpObject->OnCustomIocpMessage(msg, info);
				}
				else
				{
					//OnCustomIocpMessage(obj, msg, info);
				}
			}

			if (mLooperInternalData->mAttachThread)
			{
				//stack looper只触发，不收真正有用的消息
				return -1;
			}
		}
	} while (1);

	return -1;
}

bool Looper_Windows::PostQueuedCompletionStatus(HANDLE handle, DWORD bytes, ULONG_PTR key, LPOVERLAPPED lpOverlapped)
{
	//DV("PostQueuedCompletionStatus(handle=%d)",handle);

	bool ok = !!::PostQueuedCompletionStatus(handle, bytes, key, lpOverlapped);
	if (!ok)
	{
		DW("fail to PostQueuedCompletionStatus,error=%d", ShellTool::GetLastError());
		int x = 0;
		ASSERT(ok);
	}

	return ok;
}

}
}
