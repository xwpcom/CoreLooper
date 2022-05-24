#include "stdafx.h"
#include "core/protocol/jcp/jcp.protocol.h"

namespace Bear {
namespace Jcp {
JcpProtocol::JcpProtocol()
{
	SetObjectName("JcpProtocol");
}

void JcpProtocol::OnCreate()
{
	__super::OnCreate();
}

void JcpProtocol::sendJson(JsonObject& json)
{
	string text;
	json.printTo(text);
	text += "\r\n\r\n";

	SignalJcpWrite(this, (LPBYTE)text.c_str(), text.length());
}

void JcpProtocol::InputData(LPBYTE data, int bytes)
{
	auto ret = mInbox.Write(data, bytes);
	if (ret != bytes)
	{
		SignalJcpError(this, "inbox overflow");
		return;
	}

	mInbox.MakeSureEndWithNull();

	while (mInbox.length() > 0)
	{
		auto text = (char*)mInbox.data();
		auto end = strstr(text, "\r\n\r\n");
		if (!end)
		{
			break;
		}

		string frame(text, end - text);
		DynamicJsonBuffer jBuffer;
		auto& json = jBuffer.parseObject(frame);
		if (json.success())
		{
			auto cmd = json["cmd"].as<string>();
			SignalJcpCommand(this, cmd, json);
		}
		else
		{
			SignalJcpError(this, "invalid json frame?");
		}

		auto bytes = end + 4 - text;
		mInbox.Eat(bytes);
	}

	auto leftBytes = mInbox.length();

	if (mInbox.length() > 4 * 1024)
	{
		SignalJcpError(this, "invalid frame?");
	}

}

}
}