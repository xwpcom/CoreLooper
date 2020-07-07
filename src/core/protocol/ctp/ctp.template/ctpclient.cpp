#include "stdafx.h"
#include "core/protocol/ctp/ctpclient.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

enum
{
	eTimerTest,
};

static const char* TAG = "CtpClient";

CtpClient::CtpClient()
{
	SetObjectName("CtpClient");
}

CtpClient::~CtpClient()
{
	CommonTextProtocolFactory::Destroy(mProtocol);
	mProtocol = nullptr;
}

void CtpClient::OnCreate()
{
	__super::OnCreate();

}

void CtpClient::OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo)
{
	if (error == 0)
	{
		mProtocol = CommonTextProtocolFactory::Create();
		mProtocol->SetCB(this);

		//SetTimer(eTimerTest, 3000);
	}

	__super::OnConnect(endPoint, error, box, extraInfo);
}

//CommonTextProtocolCB#begin
void CtpClient::OnCommand(CommonTextProtocol* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody, Bundle& ackBundle, ByteBuffer& ackBody)
{

}

//收到对方的回复时调用本接口
void CtpClient::OnCommandAck(CommonTextProtocol* obj, const string& cmd, const Bundle& reqBundle, const Bundle& ackBundle, const ByteBuffer& ackBody)
{

}

//收到对方的通知时调用本接口
void CtpClient::OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body)
{

}

//协议等出错时调用本接口
//正常情况下不会触发,仅用于开发调试
void CtpClient::OnError(CommonTextProtocol* obj, int error, const string& desc)
{

}

//有数据要发给对方时，会调用本接口
void CtpClient::Output(CommonTextProtocol* obj, const ByteBuffer& data)
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
}
//CommonTextProtocolCB#end

void CtpClient::OnTimer(long id)
{
#ifdef _DEBUGx
	if (id == eTimerTest)
	{
		if (mProtocol)
		{
			Bundle bundle;
			bundle.Set("name", "xwp");
			mProtocol->AddCommand("hello", bundle);

		}

		return;
	}
#endif

	__super::OnTimer(id);
}

void CtpClient::ParseInbox()
{
	if (!mInbox.IsEmpty() && mProtocol)
	{
		KeepAlive();

		mProtocol->Input(mInbox.GetDataPointer(), mInbox.GetDataLength());
		mInbox.clear();
	}
}

}
}
}
}
}
