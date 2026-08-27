#include "stdafx.h"
#include "protocol/sctp/jpProtocol.h"
#include "protocol/sctp/sctp.h"
using namespace SCTP;

JpDemux::JpDemux()
{

}

/*
frame:
{jsonMinify...}\n
*/
void JpDemux::inputData(void* data, int bytes)
{
	mInbox.Write(data, bytes);
	mInbox.MakeSureEndWithNull();

	while (!mInbox.empty())
	{
		auto ch = *mInbox.data();
		if (ch != '{')
		{
			mInbox.Eat(1);
			continue;
		}

		auto ps = (const char*)mInbox.data();
		auto end = strchr(ps, '\n');
		if (!end)
		{
			break;
		}

		auto jsonBytes = end - ps;
		string text(ps, jsonBytes);
		mInbox.Eat(jsonBytes+1);

		onRecvJsonText(text);
	}

	if (mInbox.bytes() > 8 * 1024)
	{
		LogW("jpDemux", "invalid frame?");
		mInbox.clear();
	}
}

void JpDemux::onRecvJsonText(const string& text)
{
	StaticJsonBuffer<1024*8> jBuf;
	auto& json = jBuf.parseObject(text);
	if (json.success())
	{
		auto cmd=json["cmd"].as<string>();

		Sctp obj;
		obj.Create();
		obj.PrepareCreateOutboxData();
		obj.AddField("cmd", cmd.c_str());

		for (auto& jItem : json)
		{
			if (strcmp(jItem.key, "cmd") == 0)
			{
				continue;
			}

			obj.AddField(jItem.key, jItem.value);
		}

		int ackSeq = -1;
		const tagByteBuffer& box = obj.CreateOutboxData(&ackSeq);

		if (mFrameCB)
		{
			mFrameCB((const char*)box.mBuf, box.mBytes);
		}

	}

}

