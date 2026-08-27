#include "stdafx.h"
#include "protocol/sctp/sctpclient.h"
#include "protocol/sctp/sctp.h"
#include "protocol/sctp/jpProtocol.h"

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

	if (jpEnabled())
	{
		mJpDemux = make_shared<JpDemux>();
		mJpDemux->setFrameCB([this](const char* data, int bytes) {
			auto obj = mProtocol.lock();
			if (obj)
			{
				obj->InputData((void*)data, bytes);
			}
			});
	}

}

void SctpClient::OnRecvCommand(Sctp* obj, const char* szCommand, tagBundle* params)
{
	KeepAlive();

	if (mDumpCommand)
	{
		LogV(TAG, "%s,cmd=[%s]", __func__, szCommand);
	}
	string cmd = szCommand;

	DynamicJsonBuffer jBuffer;
	auto& root = jBuffer.createObject();

	for (int i = 0; i < params->mCount; i++)
	{
		tagKeyValue& item = params->mItems[i];
		root[item.name] = item.value;
		
		if (mDumpCommand)
		{
			LogV(TAG, "item[%02d] [%s]=[%s]", i, item.name, item.value);
		}
	}

	{
		//LogV(TAG, "%s(%s)", __func__, cmd.c_str());

		//LogV(TAG, "cmd=%s",cmd.c_str());
		auto iter = mEntries.find(cmd);
		if (iter != mEntries.end())
		{
			auto& fun = iter->second;
			(this->*fun)(cmd, root);
		}

		SignalOnCommand(this, cmd, root);
	}
}

void SctpClient::ParseInbox()
{
	auto obj = mProtocol.lock();
	if (obj)
	{
		if (jpEnabled() && mJpDemux)
		{
			mJpDemux->inputData(mInbox.data(), mInbox.length());
		}
		else
		{
			obj->InputData(mInbox.data(), mInbox.length());
		}
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

	if (jpEnabled() && mJpDemux)
	{
		DynamicJsonBuffer jBuf;
		auto& json = jBuf.createObject();

		json["cmd"]=cmd;
		if (ackSeq != -1)
		{
			json["seq"] = ackSeq;
		}

		for (map<string, string>::iterator iter = bundle.mItems.begin(); iter != bundle.mItems.end(); ++iter)
		{
			auto& name = iter->first;
			if (name != "cmd" && name != "crc")
			{
				json[name] = iter->second;
			}
		}
		
		string text;
		json.printTo(text);
		text += "\n";

		if (mChannel)
		{
			mChannel->Send((void*)text.data(), text.length());
		}
	}
	else
	{
		int eatBytes = 0;
		if (mChannel)
		{
			mChannel->Send(box.mBuf, box.mBytes);
			//SignalWrite(this, box.mBuf, box.mBytes, eatBytes);
		}
	}

	return ackSeq;
}


}
