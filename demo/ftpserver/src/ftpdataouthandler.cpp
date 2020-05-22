#include "stdafx.h"
#include "ftpdataouthandler.h"
#include "net/channel.h"
#include "datasource.h"
#include "datasink.h"

using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpDataOutHandler::FtpDataOutHandler()
{
	SetObjectName("FtpDataOutHandler");
	//DV("%s(%p)", __func__, this);
	mDataOut = true;
	mOutbox.PrepareBuf(4 * 1024);
}

FtpDataOutHandler::~FtpDataOutHandler()
{
	//DV("%s(%p)", __func__, this);
}

void FtpDataOutHandler::OnConnect(Channel*, long error, Bundle*)
{
}

void FtpDataOutHandler::OnClose(Channel*)
{
	if(!mSinkDoneFired)
	{
		mSinkDoneFired = true;
		auto obj = mDataSink.lock();
		if (obj)
		{
			SignalSinkDone(this);
		}
	}

	if (mDataEndPoint)
	{
		mDataEndPoint->Destroy();
		mDataEndPoint = nullptr;
	}
	
	Destroy();
}

void FtpDataOutHandler::OnSend(Channel*)
{
	CheckSend();
}

void FtpDataOutHandler::CheckSend()
{
	if (mHasFireSendDone)
	{
		return;
	}

	auto source = mDataSource.lock();
	while (mDataEndPoint && source)
	{
		int frameLen = mOutbox.GetActualDataLength();
		if (frameLen > 0)
		{
			LPBYTE frame = mOutbox.GetDataPointer();
			int ret = mDataEndPoint->Send(frame, frameLen);
			if (ret > 0)
			{
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

		ASSERT(mOutbox.IsEmpty());

		auto buf = mOutbox.GetNewDataPointer();
		auto bytes = mOutbox.GetTailFreeSize();
		ASSERT(bytes > 0);
		
		auto ret=source->Read(buf,bytes);
		if (ret > 0)
		{
			mOutbox.WriteDirect(ret);
			continue;
		}
		else if(ret==0)
		{
			ASSERT(!mHasFireSendDone);
			mDataEndPoint->MarkEndOfSend();
			mHasFireSendDone = true;
			SignalSendDone(this);

			Destroy();
			return;
		}

		//当前没有数据要发送
		return;
	}
}

void FtpDataOutHandler::OnReceive(Channel*)
{
	BYTE buf[4 * 1024];
	auto sink = mDataSink.lock();
	while (mDataEndPoint)
	{
		int ret = mDataEndPoint->Receive(buf, sizeof(buf));
		if (ret <= 0)
		{
			return;
		}

		if (sink)
		{
			auto bytes=sink->Write(buf, ret);
			if (bytes != ret)
			{
				Destroy();
				return;
			}
		}
	}
}

void FtpDataOutHandler::SetDataSource(shared_ptr<DataSource> obj)
{
	mDataSource = obj;

	if (obj)
	{
		CheckSend();
	}
}
