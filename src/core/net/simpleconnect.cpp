#include "stdafx.h"
#include "net/simpleconnect.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core
{
namespace Net {

SimpleConnect::SimpleConnect()
{
	mTimeOutSecond = 90;
	DV("%s,this=%p", __func__, this);
}

SimpleConnect::~SimpleConnect()
{
	DV("%s,this=%p", __func__, this);
}

int SimpleConnect::StartConnect(Bundle& bundle)
{
	if (mDataEndPoint)
	{
		//目前不支持多次调用
		ASSERT(FALSE);
		return -1;
	}

	auto obj(make_shared<TcpClient>());

	obj->SignalOnConnect.connect(this, &SimpleConnect::OnConnect);
	obj->SignalOnSend.connect(this, &SimpleConnect::OnSend);
	obj->SignalOnReceive.connect(this, &SimpleConnect::OnReceive);
	obj->SignalOnClose.connect(this, &SimpleConnect::OnClose);
	mDataEndPoint = obj;
	AddChild(obj);

	obj->Connect(bundle);

	SetTimer(mTimer_AutoClose, mTimeOutSecond * 1000);
	return 0;
}

void SimpleConnect::OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo)
{
	if (error)
	{
		DW("%s,connect fail", __func__);
		Destroy();
	}
	else
	{
		//DV("%s,connect ok", __func__);
		mConnected = true;
		mOutbox.PrepareBuf(16 * 1024);
	}
}

void SimpleConnect::OnClose(Channel*)
{
	//DV("%s,this=%p", __func__, this);
	mConnected = false;
	Destroy();
}

void SimpleConnect::OnSend(Channel*)
{
	CheckSend();
}

void SimpleConnect::OnReceive(Channel*)
{
	while (mDataEndPoint)
	{
		BYTE buf[1024*4];
		int ret = mDataEndPoint->Receive(buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			break;
		}

		buf[ret] = 0;
		//DV("recv [%s]", buf);
		auto bytes = mInbox.Write(buf, ret);
		if (bytes != ret)
		{
			DW("mInbox.len=%d overflow,ret=%d,bytes=%d,", mInbox.GetDataLength(), ret, bytes);
			Destroy();
			return;
		}

		mInbox.MakeSureEndWithNull();
		ParseInbox();
	}
}

void SimpleConnect::OnDestroy()
{
	//DV("%s,this=%p", __func__, this);

	if (mDataEndPoint)
	{
		mDataEndPoint->Destroy();
		PostDispose(mDataEndPoint);
		mConnected = false;
	}

	__super::OnDestroy();
}

void SimpleConnect::CheckSend()
{
	while (mDataEndPoint)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			OnOutboxWritable();
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
		int ret = mDataEndPoint->Send(frame, frameLen);
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

	if (mDataEndPoint)
	{
		mDataEndPoint->Close();
	}
}

void SimpleConnect::OnTimer(long id)
{
	if (id == mTimer_AutoClose)
	{
		DW("timeout auto close,this=%p", this);
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

}
}
}
