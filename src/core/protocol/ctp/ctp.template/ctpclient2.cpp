#include "stdafx.h"
#include "core/protocol/ctp/ctpclient2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

enum
{
	eTimerTest,
};

static const char* TAG = "CtpClient2";

CtpClient2::CtpClient2()
{
	SetObjectName("CtpClient2");
}

CtpClient2::~CtpClient2()
{
	CommonTextProtocolFactory2::Destroy(mProtocol);
	mProtocol = nullptr;
}

void CtpClient2::OnCreate()
{
	__super::OnCreate();
	UpdateTickAlive();
}

void CtpClient2::OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo)
{
	if (error == 0)
	{
		mProtocol = CommonTextProtocolFactory2::Create();
		mProtocol->SetCB(this);

		UpdateTickAlive();
		int second = 60;
		SetTimer(mTimer_CheckAlive, second * 1000);
	}

	__super::OnConnect(endPoint, error, box, extraInfo);
}

//CommonTextProtocolCB#begin
void CtpClient2::OnCommand(CommonTextProtocol2* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody)
{

}

//Э��ȳ���ʱ���ñ��ӿ�
//��������²��ᴥ��,�����ڿ�������
void CtpClient2::OnError(CommonTextProtocol2* obj, int error, const string& desc)
{

}

//������Ҫ�����Է�ʱ������ñ��ӿ�
void CtpClient2::Output(CommonTextProtocol2* obj, const ByteBuffer& data)
{
	int ret = mOutbox.Append(data);
	if (ret != data.GetActualDataLength())
	{
		LogW(TAG,"fail append data");
		if (mChannel)
		{
			mChannel->Close();
		}
	}
	CheckSend();
}
//CommonTextProtocolCB#end

void CtpClient2::OnTimer(long id)
{
	if (id == mTimer_CheckAlive)
	{
		auto tickNow = ShellTool::GetTickCount64();
		int second = 180;

		if (tickNow > mTickAlive + second * 1000)
		{
			LogI(TAG, "timeout,auto close");
			if (mChannel)
			{
				mChannel->Close();
			}
		}
		return;
}

	__super::OnTimer(id);
}

void CtpClient2::ParseInbox()
{
	if (!mInbox.IsEmpty() && mProtocol)
	{
		KeepAlive();

		mProtocol->Input(mInbox.GetDataPointer(), mInbox.GetDataLength());
		mInbox.clear();
	}
}

void CtpClient2::AddMessage(const string& cmd, const Bundle& bundle)
{
	if (mProtocol)
	{
		mProtocol->AddCommand(cmd, bundle);
	}
}


}
}
}
}
}
