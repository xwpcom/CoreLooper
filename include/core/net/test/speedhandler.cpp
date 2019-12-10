#include "stdafx.h"
#include "speedhandler.h"

namespace Bear {
namespace Test {
SpeedHandler::SpeedHandler()
{

}

SpeedHandler::~SpeedHandler()
{

}

void SpeedHandler::OnCreate()
{
	__super::OnCreate();

	mOutbox.PrepareBuf(1024 * 256);

	UpdateTickAlive();

	SetTimer(mTimerDump, 2000);
	SetTimer(mTimerSend, 1);
}

void SpeedHandler::OnClose(Channel*)
{
	if (mChannel)
	{
		mChannel->Destroy();
		mChannel = nullptr;
	}

	Destroy();
}

void SpeedHandler::OnSend(Channel*)
{
	CheckSend();
}

void SpeedHandler::OnReceive(Channel*)
{
	BYTE buf[4 * 1024];
	while (mChannel)
	{
		int ret = mChannel->Receive(buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			return;
		}

		mCounterUp.Add(ret);

		buf[ret] = 0;
		//DV("%s", buf);

	}
}

void SpeedHandler::OnConnect(Channel* channel, long error, Bundle*)
{
	if (error == 0)
	{
		channel->ConfigRecvBuf(128 * 1024);
		channel->ConfigSendBuf(128 * 1024);

		UpdateTickAlive();
	}
}


void SpeedHandler::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void SpeedHandler::CheckSend()
{
	while (mChannel)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
		int ret = mChannel->Send(frame, frameLen);
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

void SpeedHandler::OnTimer(long id)
{
	if (id == mTimerSend)
	{
		auto now = ShellTool::GetCurrentTimeMs();
		string msg = StringTool::Format("%02d:%02d:%02d.%03d", now.hour, now.minute, now.second, now.ms);

		while (mOutbox.GetFreeSize() >= (int)msg.length() + 1)
		{
			mOutbox.Write(msg);
		}

		CheckSend();

		return;
	}
	else if (id == mTimerDump)
	{

		double upSpeed = 0;
		double downSpeed = 0;
		{
			DWORD count, interval;
			mCounterUp.Reset(count, interval);
			if (interval > 0)
			{
				upSpeed = count / interval;
			}
		}

		{
			DWORD count, interval;
			mCounterDown.Reset(count, interval);
			if (interval > 0)
			{
				downSpeed = count / interval;
			}
		}

		DV("up=%.1f,down=%.1f", upSpeed, downSpeed);


		return;
	}

	__super::OnTimer(id);
}

}
}
