#include "stdafx.h"
#include "core/protocol/ctp/CtpHandler2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

static const char* TAG = "CtpHandler2";

CtpHandler2::CtpHandler2()
{
	SetObjectName("CtpHandler2");
}

CtpHandler2::~CtpHandler2()
{
	CommonTextProtocolFactory2::Destroy(mProtocol);
}

void CtpHandler2::OnCreate()
{
	__super::OnCreate();

	mOutbox.PrepareBuf(1024 * 16);

	UpdateTickAlive();
}

void CtpHandler2::OnClose(Channel*)
{
	PostDispose(mDataEndPoint);
	Destroy();
}

void CtpHandler2::OnSend(Channel*)
{
	CheckSend();
}

void CtpHandler2::OnReceive(Channel*)
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

		if (mProtocol)
		{
			mProtocol->Input(buf, ret);
		}
	}
}

void CtpHandler2::OnConnect(Channel*, long error, Bundle*)
{
	if (error == 0)
	{
		if (mProtocol)
		{
			mProtocol->ResetX();
		}
		else
		{
			mProtocol = CommonTextProtocolFactory2::Create();

			mProtocol->SetCB(this);

			OnProtocolCreated();
		}

		int second = 60;
		SetTimer(mTimer_CheckAlive, second * 1000);
		UpdateTickAlive();
	}
}


//CommonTextProtocolCB#begin
void CtpHandler2::OnCommand(CommonTextProtocol2* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody)
{
	int x = 0;
}

//协议等出错时调用本接口
//正常情况下不会触发,仅用于开发调试
void CtpHandler2::OnError(CommonTextProtocol2* obj, int error, const string& desc)
{

}

//有数据要发给对方时，会调用本接口
void CtpHandler2::Output(CommonTextProtocol2* obj, const ByteBuffer& data)
{
	int ret = mOutbox.Append(data);
	if (ret != data.GetActualDataLength())
	{
		LogW(TAG, "fail append data(%s)", GetObjectName().c_str());
		if (mDataEndPoint)
		{
			mDataEndPoint->Close();
		}
		Destroy();
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

void CtpHandler2::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void CtpHandler2::CheckSend()
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

void CtpHandler2::OnTimer(long id)
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

int CtpHandler2::AddCommand(const string& cmd, const Bundle& bundle)
{
	if (mProtocol)
	{
		return mProtocol->AddCommand(cmd, bundle);
	}

	return -1;
}


}
}
}
}
}
