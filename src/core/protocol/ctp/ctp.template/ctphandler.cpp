#include "stdafx.h"
#include "core/protocol/ctp/CtpHandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

static const char* TAG = "CtpHandler";

CtpHandler::CtpHandler()
{
	SetObjectName("CtpHandler");
}

CtpHandler::~CtpHandler()
{
	CommonTextProtocolFactory::Destroy(mProtocol);
}

void CtpHandler::OnCreate()
{
	__super::OnCreate();

	mOutbox.PrepareBuf(1024 * 16);

	UpdateTickAlive();
}

void CtpHandler::OnClose(Channel*)
{
	PostDispose(mDataEndPoint);
	Destroy();
}

void CtpHandler::OnSend(Channel*)
{
	CheckSend();
}

void CtpHandler::OnReceive(Channel*)
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

		if (mProtocol)
		{
			mProtocol->Input(buf, ret);
		}
	}
}

void CtpHandler::OnConnect(Channel*, long error, Bundle*)
{
	if (error == 0)
	{
		if (mProtocol)
		{
			mProtocol->ResetX();
		}
		else
		{
			mProtocol = CommonTextProtocolFactory::Create();
			mProtocol->SetCB(this);
		}

		int second = 60;
		SetTimer(mTimer_CheckAlive, second * 1000);
		UpdateTickAlive();
	}
}


//CommonTextProtocolCB#begin
void CtpHandler::OnCommand(CommonTextProtocol* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody, Bundle& ackBundle, ByteBuffer& ackBody)
{
	UpdateTickAlive();

	{
		ackBundle.Set("error", -1);
		ackBundle.Set("errorName", "EUnknownCommand");
	}

	int x = 0;
}

//收到对方的回复时调用本接口
void CtpHandler::OnCommandAck(CommonTextProtocol* obj, const string& cmd, const Bundle& reqBundle, const Bundle& ackBundle, const ByteBuffer& ackBody)
{
	UpdateTickAlive();
	int x = 0;
}

//收到对方的通知时调用本接口
void CtpHandler::OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body)
{
	UpdateTickAlive();
	//DV("notify=%s", cmd.c_str());
}

//协议等出错时调用本接口
//正常情况下不会触发,仅用于开发调试
void CtpHandler::OnError(CommonTextProtocol* obj, int error, const string& desc)
{

}

//有数据要发给对方时，会调用本接口
void CtpHandler::Output(CommonTextProtocol* obj, const ByteBuffer& data)
{
	int ret = mOutbox.Append(data);
	if (ret != data.GetActualDataLength())
	{
		LogW(TAG,"fail append data");
		if (mDataEndPoint)
		{
			mDataEndPoint->Close();
		}
	}
	CheckSend();

	/*
	if (mDumpFile.IsOpen())
	{
	mDumpFile.Write(data.GetDataPointer(), data.GetActualDataLength());
	}
	//*/
}

//CommonTextProtocolCB#end

void CtpHandler::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void CtpHandler::CheckSend()
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

void CtpHandler::OnTimer(long id)
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

#ifdef _DEBUGx
		{
			if (mProtocol)
			{
				Bundle bundle;
				bundle.Set("name", "bear");
				mProtocol->AddCommand("test", bundle);
			}
		}
#endif

		return;
	}

	__super::OnTimer(id);
}


}
}
}
}
}
