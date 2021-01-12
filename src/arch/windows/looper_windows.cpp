#include "stdafx.h"
#include "looper_windows.h"
#include "thread/autolock.h"
#include "iocpobject.h"
#include "iocontext.h"
#include "looper/handler.h"
#include "looper/looper.h"
#include "../../core/looper/handlerinternaldata.h"
#include "../../core/looper/looperinternaldata.h"
#include "../../core/looper/timermanager.h"

#pragma comment(lib, "WS2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Bear::Core::Net;

namespace Bear {
namespace Core
{
static const char* TAG = "Looper";

Looper_Windows::Looper_Windows()
{
	SetObjectName("Looper");
	mThreadName = GetObjectName();

	{
		static bool first = true;
		if (first)
		{
			first = false;
			SockTool::InitSockTool();
		}
	}
}

Looper_Windows::~Looper_Windows()
{
}

int Looper_Windows::StartHelper(bool newThread)
{
	if (mLooperInternalData->mLooperRunning)
	{
		LogW(TAG,"mLooperRunning=%d,CurrentLooper=%p", mLooperInternalData->mLooperRunning, CurrentLooper());
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

#define _CONFIG_SMART_TLS_LOOPER

#ifdef _CONFIG_SMART_TLS_LOOPER

class SmartTlsLooperManager
{
	static void clear();
public:
	shared_ptr<Looper_Windows> PopTlsLooper()
	{
		{
			AutoLock lock(&mCS);
			if (!mLoopers.empty())
			{
				auto obj = mLoopers.back();
				mLoopers.pop_back();
				return obj;
			}
		}

		{
			static bool first = true;
			if (first)
			{
				first = false;

				atexit(SmartTlsLooperManager::clear);
			}

		}

		auto looper = make_shared<Looper_Windows >();
		looper->mThreadId = ShellTool::GetCurrentThreadId();
		looper->mLooperInternalData->mAttachThread = true;
		looper->mLooperHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

		return looper;
	}
	
	void PushTlsLooper(shared_ptr<Looper_Windows> obj)
	{
		AutoLock lock(&mCS);
		mLoopers.push_back(obj);
	}

protected:
	CriticalSection mCS;
	list<shared_ptr<Looper_Windows>> mLoopers;
};

static SmartTlsLooperManager gTlsLooperManager;

void SmartTlsLooperManager::clear()
{
	AutoLock lock(&gTlsLooperManager.mCS);
	gTlsLooperManager.mLoopers.clear();
}

class SmartTlsLooper
{
public:														   
	SmartTlsLooper(LooperImpl* obj)
	{
		mLooper = gTlsLooperManager.PopTlsLooper();
		mLooper->mThreadId = ShellTool::GetCurrentThreadId();

		mLooperImpl = obj;
		mLooperImpl->SetCurrentLooper(mLooper.get());
	}

	~SmartTlsLooper()
	{
		gTlsLooperManager.PushTlsLooper(mLooper);
		mLooperImpl->SetCurrentLooper(nullptr);
	}

	LooperImpl* mLooperImpl;
	shared_ptr<Looper_Windows> mLooper;
};
#endif

void Looper_Windows::_StackLooperSendMessage(tagLoopMessageInternal& loopMsg)
{
#ifdef _CONFIG_SMART_TLS_LOOPER
	SmartTlsLooper obj(this);
	sendMessageHelper(loopMsg, *obj.mLooper.get());
#else
	//构造一个stack looper来实现sendMessage
	auto looper = make_shared<Looper_Windows >();
	SetCurrentLooper(looper.get());
	looper->mThreadId = ShellTool::GetCurrentThreadId();
	looper->mLooperInternalData->mAttachThread = true;
	looper->mLooperHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	sendMessageHelper(loopMsg, *looper.get());
	SetCurrentLooper(nullptr);
	looper = nullptr;
#endif
}

int Looper_Windows::getMessage(tagLoopMessageInternal& msg)
{
	mLooperTick = ShellTool::GetTickCount64();

	do
	{
		{
			AutoLock lock(&mLooperInternalData->mMessageLock);
			auto& msgList = mLooperInternalData->mMessageList;
			if (!msgList.empty())
			{
				msg = msgList.front();
				msgList.pop_front();
				mLastIoTick = mLooperTick;
				return 0;
			}
		}

		if (mLooperInternalData->mBMQuit && CanQuitLooperNow())
		{
			return -1;
		}

		DWORD msDelayNext = INFINITE;
		ProcessTimer(msDelayNext, mLooperTick - mLastIoTick);
		msDelayNext = MAX(1, msDelayNext);

		{
			//ASSERT(msDelayNext);//如果为0，会形成busy loop,占用大量cpu

#if 1
			BOOL fAlertable = FALSE;
			ULONG count = 0;
			OVERLAPPED_ENTRY items[100];
			BOOL ok=GetQueuedCompletionStatusEx(mLooperHandle
				,items
				,_countof(items)
				,&count
				,msDelayNext
				,fAlertable
			);

			if (!ok)
			{
				return 0;
			}
			
			if (count>0 && mTimerManager)
			{
				mLastIoTick = mLooperTick;
			}

			if (count > 1)
			{
				//LogV(TAG, "iocp finish count = %d",(int)count);
			}

			for (ULONG i = 0; i < count; i++)
			{
				auto& item = items[i];
				auto ptr = item.lpCompletionKey;
				auto ov = item.lpOverlapped;
				auto bytes = item.dwNumberOfBytesTransferred;

				if (ptr > 0xFFFF)
				{
					ASSERT(ov);

					IocpObject* obj = (IocpObject*)ptr;
					IoContext* context = CONTAINING_RECORD(ov, IoContext, mOV);
					obj->DispatchIoContext(context, bytes);
				}
				else if (ptr && ptr <= 0xFFFF)
				{
					UINT msg = (UINT)ptr;
					LPVOID info = (LPVOID)(ULONGLONG)bytes;
					LPVOID obj = (LPVOID)ov;

					if (obj > (LPVOID)0xFFFF)
					{
						IocpObject* iocpObject = (IocpObject*)obj;
						iocpObject->OnCustomIocpMessage(msg, info);
					}
					else
					{
						//OnCustomIocpMessage(obj, msg, info);
					}
				}
			}

#else

			BOOL ret = FALSE;

			DWORD bytes = 0;
			ULONG_PTR ptr = NULL;
			LPOVERLAPPED ov = NULL;
			ret = GetQueuedCompletionStatus(mLooperHandle, &bytes, &ptr, &ov, msDelayNext);
			if (ptr && mTimerManager)
			{
				mLastIoTick = mLooperTick;
			}
			else
			{
				int x = 0;
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
#endif

			if (mLooperInternalData->mAttachThread)
			{
				/* stack looper只触发，不收真正有用的消息 */
				
				return -1;
			}
		}
	} while (1);

	return -1;
}

bool Looper_Windows::PostQueuedCompletionStatus(HANDLE handle, DWORD bytes, ULONG_PTR key, LPOVERLAPPED lpOverlapped)
{
	bool ok = !!::PostQueuedCompletionStatus(handle, bytes, key, lpOverlapped);
	if (!ok)
	{
		LogW(TAG,"fail to PostQueuedCompletionStatus,error=%d", ShellTool::GetLastError());
		int x = 0;
		ASSERT(ok);
	}

	return ok;
}

}
}
