#include "stdafx.h"
#include "ftpdatahandler.h"
#include "net/channel.h"
#include "datasource.h"
#include "datasink.h"

using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpDataHandler::FtpDataHandler()
{
	SetObjectName("FtpDataHandler");
}

FtpDataHandler::~FtpDataHandler()
{
	//DV("%s(%p)", __func__, this);
}

void FtpDataHandler::OnConnect(Channel*, long error, Bundle*)
{
}

void FtpDataHandler::OnClose(Channel*)
{
	if (!mDataOut)
	{
		if (!mSinkDoneFired)
		{
			mSinkDoneFired = true;
			SignalSinkDone(this);
		}
	}

	PostDispose(mDataEndPoint);
	//PostDispose(mSelfRef);

	Destroy();
}

void FtpDataHandler::OnSend(Channel*)
{
	CheckSend();
}

void FtpDataHandler::CheckSend()
{
}

void FtpDataHandler::OnReceive(Channel*)
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

void FtpDataHandler::SetDataSource(shared_ptr<DataSource> obj)
{
}

void FtpDataHandler::SetDataSink(shared_ptr<DataSink> obj)
{
}

void FtpDataHandler::OnDataChanged(DataSource *source)
{
}
