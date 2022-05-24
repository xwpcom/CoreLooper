#include "stdafx.h"
#include "libhttp/Telnet.Handler.h"

namespace Bear {
namespace Telnet {

static const char* TAG = "TelnetHandler";

TelnetHandler::TelnetHandler()
{
	SetObjectName("TelnetHandler");

	LogV(mTag, "%s(%p)", __func__, this);
}

TelnetHandler::~TelnetHandler()
{
	LogV(mTag, "%s(%p)", __func__, this);
}

void TelnetHandler::OnCreate()
{
	__super::OnCreate();

	mOutbox.PrepareBuf(1024 * 16);

	UpdateTickAlive();
}

void TelnetHandler::OnDestroy()
{
	__super::OnDestroy();

	auto obj = mBuddy.lock();
	if (obj)
	{
		obj->Destroy();
	}
}

void TelnetHandler::OnClose(Channel*)
{
	PostDispose(mDataEndPoint);
	Destroy();
}

void TelnetHandler::OnSend(Channel*)
{
	CheckSend();
}

void TelnetHandler::OnReceive(Channel*)
{
	BYTE buf[4 * 1024];
	while (mDataEndPoint)
	{
		int ret = mDataEndPoint->Receive(buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			return;
		}

		buf[ret] = 0;

		auto bytes = mInbox.Write(buf, ret);
		if (bytes != ret)
		{
			LogW(TAG, "mInbox.len=%d overflow,ret=%d,bytes=%d,", mInbox.GetDataLength(), ret, bytes);
			Destroy();
			return;
		}

		mInbox.MakeSureEndWithNull();
		ParseInbox();

		UpdateTickAlive();
	}
}

void TelnetHandler::OnConnect(Channel*, long error, Bundle*)
{
	if (error == 0)
	{
		mConnected = true;

		int second = 60;
		SetTimer(mTimer_CheckAlive, second * 1000);
		UpdateTickAlive();
	}
}

void TelnetHandler::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void TelnetHandler::CheckSend()
{
	while (mDataEndPoint)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
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
}

void TelnetHandler::OnTimer(long id)
{
	if(id == mTimer_CheckAlive)
	{
		auto tickNow = ShellTool::GetTickCount64();
		int second = 180;

		if (tickNow > mTickAlive + second * 1000)
		{
			LogI(TAG, "timeout,auto close");
			if (mDataEndPoint)
			{
				mDataEndPoint->Close();
			}
		}
		return;
	}

	__super::OnTimer(id);
}

int TelnetHandler::sendData(LPBYTE data, int bytes)
{
	auto ret = mOutbox.Write(data, bytes);
	if (ret != bytes)
	{
		LogW(mTag, "%s fail,bytes=%d,ret=%d", __func__, bytes, ret);
		Destroy();
		return ret;
	}

	CheckSend();

	return ret;
}

void TelnetHandler::setBuddy(weak_ptr< TelnetHandler> wobj)
{
	mBuddy = wobj;

	auto obj = wobj.lock();
	if (obj)
	{
		obj->checkSendCacheData();
	}
}

void TelnetHandler::ParseInbox()
{
	auto obj = mBuddy.lock();
	if (obj && obj->IsConnected())
	{
		checkSendCacheData();

		const auto bytes = mInbox.length();
		auto ret = obj->sendData(mInbox.data(), bytes);
		if (ret != bytes)
		{
			mInbox.clear();
			Destroy();
			return;
		}
	}
	else
	{
		const auto bytes = mInbox.length();
		auto ret = mInboxCache.Append(mInbox);
		if (ret != mInbox.length())
		{
			LogW(mTag, "%s fail,bytes=%d,ret=%d", __func__, bytes, ret);
			mInbox.clear();
			Destroy();
			return;
		}
	}

	mInbox.clear();
}

void TelnetHandler::checkSendCacheData()
{
	if (!mInboxCache.empty())
	{
		auto obj = mBuddy.lock();
		if (obj && obj->IsConnected())
		{
			const auto bytes = mInboxCache.length();
			auto ret = obj->sendData(mInboxCache.data(), bytes);
			if (ret == bytes)
			{
				mInboxCache.clear();
			}
			else
			{
				mInbox.clear();
				Destroy();
				return;
			}
		}
	}
}


}
}
