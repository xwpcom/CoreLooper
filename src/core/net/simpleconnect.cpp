﻿#include "stdafx.h"
#include "net/simpleconnect.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core{
namespace Net {

#define TAG "SimpleConnect"

SimpleConnect::SimpleConnect()
{
	SetObjectName("SimpleConnect");
	mTimeOutSecond = 90;
	//LogV(TAG, "%s,this=%p", __func__, this);
	Profiler profile("SimpleConnect.ctor", 0);
}

SimpleConnect::~SimpleConnect()
{
	//LogV(TAG,"%s,this=%p", __func__, this);
	Profiler profile("SimpleConnect.dtor", 0);
}

int SimpleConnect::StartConnect(Bundle& bundle)
{
	if (IsDestroyed())
	{
		return -1;
	}

	if (mChannel)
	{
		//目前不支持多次调用
		ASSERT(FALSE);
		return -1;
	}

	auto obj(make_shared<TcpClient>());
	if (mUseTls)
	{
		obj->EnableTls();
	}

	obj->SignalOnConnect.connect(this, &SimpleConnect::OnConnect);
	obj->SignalOnSend.connect(this, &SimpleConnect::OnSend);
	obj->SignalOnReceive.connect(this, &SimpleConnect::OnReceive);
	obj->SignalOnClose.connect(this, &SimpleConnect::OnClose);
	mChannel = obj;
	AddChild(obj);

	if (mVerbose)
	{
		//LogV(TAG, "enable verbose#1");
		obj->EnableVerbose();
	}
	else
	{
		//LogV(TAG, "disable verbose#1,obj=%p",obj.get());
		obj->DisableVerbose();
	}

	obj->Connect(bundle);

	mAddress = bundle.GetString("address") + ":" + bundle.GetString("port");

	if (mTimeOutSecond > 0)
	{
		SetTimer(mTimer_AutoClose, mTimeOutSecond * 1000);
	}

	return 0;
}

void SimpleConnect::OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo)
{
	if (error)
	{
		if (mVerbose)
		{
			LogW(TAG, "%p connect [%s] fail,error=%d(%s)", this, mAddress.c_str(), error, SockTool::GetErrorDesc(error));
		}

		Destroy();
	}
	else
	{
		if (mVerbose)
		{
			LogV(TAG, "%s,connect [%s] ok", __func__, mAddress.c_str());
		}

		mConnected = true;
		mOutbox.PrepareBuf(16 * 1024);
	}

	SignalConnectAck(this, error);
}

void SimpleConnect::OnClose(Channel*)
{
	if (mVerbose)
	{
		LogV(TAG, "%s,this=%p", __func__, this);
	}

	mConnected = false;
	Destroy();
}

void SimpleConnect::OnSend(Channel*)
{
	CheckSend();
}

void SimpleConnect::OnReceive(Channel*)
{
	//在下面的ParseInbox可能导致清空mChannel,
	//引入局部变量channel可保证channel在while循环中一直有效

	auto channel = mChannel;

	while (channel)
	{
		BYTE buf[1024*4];
		int ret = channel->Receive(buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			break;
		}

		buf[ret] = 0;
		//DV("recv [%s]", buf);
		auto bytes = mInbox.Write(buf, ret);
		if (bytes != ret)
		{
			LogW(TAG,"mInbox.len=%d overflow,ret=%d,bytes=%d,", mInbox.GetDataLength(), ret, bytes);
			Destroy();
			return;
		}

		mInbox.MakeSureEndWithNull();
		ParseInbox();
	}
}

void SimpleConnect::OnDestroy()
{
	if (mVerbose)
	{
		LogV(TAG, "%s,this=%p", __func__, this);
	}

	if (mChannel)
	{
		mChannel->Destroy();
		PostDispose(mChannel);
		mConnected = false;
	}

	SignalDestroy(this);

	__super::OnDestroy();
}

void SimpleConnect::CheckSend()
{
	if (mVerbose)
	{
		//LogV(TAG, "%s", __func__);
	}

	auto channel = mChannel;

	while (channel)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			OnOutboxWritable();
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
		int ret = channel->Send(frame, frameLen);
		if (ret > 0)
		{
			SignalSendOut(this, frame, frameLen);

			mOutbox.Eat(ret);
			if (ret < frameLen)
			{
				//只发了一部分,mOutbox中没发完的数据下次会再发送
				return;
			}
		}
		else
		{
			//发送出错
			return;
		}
	}
}

void SimpleConnect::Close()
{
	ASSERT(IsMyselfThread());

	if (mVerbose)
	{
		LogV(TAG, "%s,this=%p", __func__, this);
	}

	if (mChannel)
	{
		mChannel->Close();
	}
}

void SimpleConnect::OnTimer(long id)
{
	if (id == mTimer_AutoClose)
	{
		if(mVerbose)
		{
			LogV(TAG, "%s timeout auto close,this=%p,mTimeOutSecond=%d", GetObjectName().c_str(), this, mTimeOutSecond);
		}
		
		Destroy();
		return;
	}

	__super::OnTimer(id);
}

void SimpleConnect::DelayAutoClose(int ms)
{
	KillTimer(mTimer_AutoClose);

	if (ms)
	{
		SetTimer(mTimer_AutoClose, ms);
	}
}

//seconds为0时禁用超时
void SimpleConnect::SetTimeout(int seconds)
{
	mTimeOutSecond = seconds;

	if (IsCreated())
	{
		KillTimer(mTimer_AutoClose);

		if (mTimeOutSecond > 0)
		{
			SetTimer(mTimer_AutoClose, mTimeOutSecond * 1000);
		}
	}
}


}
}
}
