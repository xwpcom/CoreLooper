#include "stdafx.h"
#include "core/protocol/ctp/ctpclient2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

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
}

void CtpClient2::OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo)
{
	if (error == 0)
	{
		mProtocol = CommonTextProtocolFactory2::Create();
		mProtocol->SetCB(this);
	}

	__super::OnConnect(endPoint, error, box, extraInfo);
}

//CommonTextProtocolCB#begin
void CtpClient2::OnCommand(CommonTextProtocol2* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody)
{

}

//协议等出错时调用本接口
//正常情况下不会触发,仅用于开发调试
void CtpClient2::OnError(CommonTextProtocol2* obj, int error, const string& desc)
{

}

//有数据要发给对方时，会调用本接口
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
