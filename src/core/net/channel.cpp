#include "stdafx.h"
#include "channel.h"
#include "../looper/message.inl"

namespace Bear {
namespace Core
{
namespace Net {
static const char* TAG = "Channel";

Channel::Channel()
{
	//DV("%s,this=0x%08x", __func__, this);

	mTickLastRecv = 0;
	mTickLastSend = 0;

	mDataTimeout = 0;//默认不超时

#ifdef _DEBUG
	mDataTimeout = 0;
#else
	mDataTimeout = 10 * 60 * 1000;
#endif

	SetObjectName("Channel");

	mTimeCreate = ShellTool::GetLocalTime();
}

Channel::~Channel()
{
	//DV("%s,this=0x%08x", __func__, this);
}

void Channel::OnCreate()
{
	SetTimer(mTimerCheckAlive,60 * 1000);
	UpdateRecvTick();//创建数据节点即认为收到一个信号,所以更新recv tick
	__super::OnCreate();
}

void Channel::OnDestroy()
{
	Close();

	__super::OnDestroy();
}

void Channel::OnTimer(long timerId)
{
	if(timerId == mTimerCheckAlive)
	{
		if (mDataTimeout != 0)
		{
			//一般应该以收到新数据时才刷新保活tick
			//特例:在http post上传大文件时，可能很长时间只发数据，没收数据
			ULONGLONG tickNow = ShellTool::GetTickCount64();
			if (tickNow >= mTickLastRecv + mDataTimeout && tickNow >= mTickLastSend + mDataTimeout)
			{
				LogW(TAG,"%s(0x%08x) timeout,auto close it,mTimeCreate=%d.%02d.%02d %02d:%02d:%02d"
					, GetObjectName().c_str()
					, this
					, mTimeCreate.tm_year + 1900
					, mTimeCreate.tm_mon + 1
					, mTimeCreate.tm_mday
					, mTimeCreate.tm_hour
					, mTimeCreate.tm_min
					, mTimeCreate.tm_sec
				);

				Close();
			}
		}
		return;
	}

	__super::OnTimer(timerId);
}

void Channel::UpdateRecvTick()
{
	mTickLastRecv = ShellTool::GetTickCount64();
}

void Channel::UpdateSendTick()
{
	mTickLastSend = ShellTool::GetTickCount64();
}

LRESULT Channel::HandleClose(WPARAM wp, LPARAM lp)
{
	UNUSED(wp);
	UNUSED(lp);

	DW("%s", __func__);
	OnClose();
	return 0;
}

void Channel::Close()
{
	if (IsMyselfThread())
	{
		Close_Impl();
		return;
	}

	sendMessage(mMessageClose);
}

void Channel::Close_Impl()
{
	ASSERT(IsMyselfThread());
}
}
}
}
