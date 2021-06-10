#include "stdafx.h"
#include "protocol/sctp/sctpclient.h"
#include "protocol/sctp/sctp.h"

namespace SCTP {
static const char* TAG = "sctpClient";

SctpClient::SctpClient()
{
	SetObjectName("SctpClient");
}

void SctpClient::OnCreate()
{
	__super::OnCreate();

	auto obj = make_shared<Sctp>();
	AddChild(obj);
	obj->Create();//注意要调用Create()才能初始化sctp功能

	mProtocol = obj;
	obj->SignalOnRecvCommand.connect(this, &SctpClient::OnRecvCommand);

	InitEntries();

}

void SctpClient::OnRecvCommand(Sctp* obj, const char* szCommand, tagBundle* params)
{
	LogV(TAG, "%s,cmd=[%s]", __func__, szCommand);
	string cmd = szCommand;

	DynamicJsonBuffer jBuffer;
	auto& root = jBuffer.createObject();

	for (int i = 0; i < params->mCount; i++)
	{
		tagKeyValue* item = params->mItems;
		root[item->name] = item->value;
	}

	{
		LogV(TAG, "%s(%s)", __func__, cmd.c_str());

		//LogV(TAG, "cmd=%s",cmd.c_str());
		auto iter = mEntries.find(cmd);
		if (iter != mEntries.end())
		{
			auto& fun = iter->second;
			(this->*fun)(cmd, root);
		}
	}

}

void SctpClient::ParseInbox()
{
	auto obj = mProtocol.lock();
	if (obj)
	{
		obj->InputData(mInbox.data(), mInbox.length());
	}

	mInbox.clear();
}

void SctpClient::InitEntries()
{

}

int SctpClient::AddCommand(const string& cmd, Bundle& bundle)
{
	if (!mConnected || cmd.empty())
	{
		LogV(TAG, "skip %s,mConnected=%d,cmd=[%s]", __func__, mConnected, cmd.c_str());
		return -1;
	}

	auto obj = mProtocol.lock();
	if (!obj)
	{
		return -1;
	}

	auto& sctp = *obj.get();

	sctp.PrepareCreateOutboxData();

	sctp.AddField("cmd", cmd.c_str());
	for (map<string, string>::iterator iter = bundle.mItems.begin(); iter != bundle.mItems.end(); ++iter)
	{
		sctp.AddField(iter->first.c_str(), iter->second.c_str());
	}

	int ackSeq = -1;
	const tagByteBuffer& box = sctp.CreateOutboxData(&ackSeq);
	int eatBytes = 0;
	if (mChannel)
	{
		mChannel->Send(box.mBuf, box.mBytes);
		//SignalWrite(this, box.mBuf, box.mBytes, eatBytes);
	}

	return ackSeq;
}


}
