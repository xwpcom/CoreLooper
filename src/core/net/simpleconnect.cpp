#include "stdafx.h"
#include "net/simpleconnect.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core{
namespace Net {

#define TAG "SimpleConnect"

SimpleConnect::SimpleConnect()
{
	mTimeOutSecond = 90;
	//LogV(TAG, "%s,this=%p", __func__, this);
}

SimpleConnect::~SimpleConnect()
{
	//LogV(TAG,"%s,this=%p", __func__, this);
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
#ifdef _MSC_VER
	if (mUseTls)
	{
		obj->EnableTls();
	}
#endif

	obj->SignalOnConnect.connect(this, &SimpleConnect::OnConnect);
	obj->SignalOnSend.connect(this, &SimpleConnect::OnSend);
	obj->SignalOnReceive.connect(this, &SimpleConnect::OnReceive);
	obj->SignalOnClose.connect(this, &SimpleConnect::OnClose);
	mChannel = obj;
	AddChild(obj);

	obj->Connect(bundle);

	SetTimer(mTimer_AutoClose, mTimeOutSecond * 1000);
	return 0;
}

void SimpleConnect::OnConnect(Channel *endPoint, long error, ByteBuffer*, Bundle* extraInfo)
{
	if (error)
	{
		LogW(TAG,"%p connect fail,error=%d(%s)",this, error,SockTool::GetErrorDesc(error));

		Destroy();
	}
	else
	{
		//DV("%s,connect ok", __func__);
		mConnected = true;
		mOutbox.PrepareBuf(16 * 1024);
	}

	SignalConnectAck(this, error);
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
	while (mChannel)
	{
		BYTE buf[1024*4];
		int ret = mChannel->Receive(buf, sizeof(buf) - 1);
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
	//LogV(TAG,"%s,this=%p", __func__, this);

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
	while (mChannel)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			OnOutboxWritable();
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
		int ret = mChannel->Send(frame, frameLen);
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

	if (mChannel)
	{
		mChannel->Close();
	}
}

void SimpleConnect::OnTimer(long id)
{
	if (id == mTimer_AutoClose)
	{
		LogW(TAG,"timeout auto close,this=%p", this);
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
