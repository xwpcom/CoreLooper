#include "stdafx.h"
#include "ftpdatainhandler.h"
#include "net/channel.h"
#include "datasource.h"
#include "datasink.h"

using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpDataInHandler::FtpDataInHandler()
{
	SetObjectName("FtpDataInHandler");
	DV("%s(%p)", __func__, this);
	mDataOut = false;
}


FtpDataInHandler::~FtpDataInHandler()
{
	DV("%s(%p)", __func__, this);
}

void FtpDataInHandler::OnConnect(Channel*, long error, Bundle*)
{
}

void FtpDataInHandler::OnClose(Channel* obj)
{
	if(!mSinkDoneFired)
	{
		mSinkDoneFired = true;
		SignalSinkDone(this);
	}

	__super::OnClose(obj);
}

void FtpDataInHandler::OnSend(Channel*)
{
	CheckSend();
}

void FtpDataInHandler::CheckSend()
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

void FtpDataInHandler::OnReceive(Channel*)
{
	BYTE buf[4 * 1024];
	while (mDataEndPoint)
	{
		int ret = mDataEndPoint->Receive(buf, sizeof(buf));
		if (ret <= 0)
		{
			return;
		}

		auto obj = mDataSink.lock();
		if (obj)
		{
			obj->Write(buf, ret);
		}
	}
}

void FtpDataInHandler::SetDataSource(shared_ptr<DataSource> obj)
{
	mDataSource = obj;

	if (obj)
	{
		CheckSend();
	}
}

void FtpDataInHandler::SetDataSink(shared_ptr<DataSink> obj)
{
	mDataSink = obj;
}

void FtpDataInHandler::OnDataChanged(DataSource *source)
{
	CheckSend();
}
