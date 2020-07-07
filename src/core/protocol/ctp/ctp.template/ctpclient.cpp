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

//�յ��Է��Ļظ�ʱ���ñ��ӿ�
void CtpClient::OnCommandAck(CommonTextProtocol* obj, const string& cmd, const Bundle& reqBundle, const Bundle& ackBundle, const ByteBuffer& ackBody)
{

}

//�յ��Է���֪ͨʱ���ñ��ӿ�
void CtpClient::OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body)
{

}

//Э��ȳ���ʱ���ñ��ӿ�
//��������²��ᴥ��,�����ڿ�������
void CtpClient::OnError(CommonTextProtocol* obj, int error, const string& desc)
{

}

//������Ҫ�����Է�ʱ������ñ��ӿ�
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
