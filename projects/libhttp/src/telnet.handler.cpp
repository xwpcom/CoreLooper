#include "stdafx.h"
#include "libhttp/Telnet.Handler.h"

namespace Bear {
namespace Telnet {

static const char* TAG = "TelnetHandler";

TelnetHandler::TelnetHandler()
{
	SetObjectName("TelnetHandler");
}

TelnetHandler::~TelnetHandler()
{
}

void TelnetHandler::OnCreate()
{
	__super::OnCreate();

	mOutbox.PrepareBuf(1024 * 16);

	UpdateTickAlive();
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
		UpdateTickAlive();
	}
}

void TelnetHandler::OnConnect(Channel*, long error, Bundle*)
{
	if (error == 0)
	{
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


}
}
