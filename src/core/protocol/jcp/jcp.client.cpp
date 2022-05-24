#include "stdafx.h"
#include "core/protocol/jcp/jcp.client.h"
#include "core/protocol/jcp/jcp.protocol.h"

#if defined _CONFIG_JCP
namespace Bear {
namespace Jcp {
JcpClient::JcpClient()
{
	SetObjectName("JcpClient");
}

void JcpClient::OnCreate()
{
	__super::OnCreate();

	auto obj = make_shared<JcpProtocol>();
	AddChild(obj);

	mProtocol = obj;
	obj->SignalJcpCommand.connect(this, &JcpClient::OnJcpCommand);
	obj->SignalJcpWrite.connect(this, &JcpClient::OnJcpWrite);
	obj->SignalJcpError.connect(this, &JcpClient::OnJcpError);

	InitEntries();

}

void JcpClient::OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo)
{
	__super::OnConnect(endPoint, error, box, extraInfo);
}

void JcpClient::sendJson(JsonObject& json)
{
	auto obj = mProtocol.lock();
	if (obj)
	{
		obj->sendJson(json);
	}
}

void JcpClient::OnJcpError(Handler*, const string& desc)
{
	LogW(mTag, "%s(%s)", __func__, desc.c_str());
}

void JcpClient::OnJcpWrite(Handler*, LPBYTE data, int bytes)
{
	if (mChannel)
	{
		mChannel->Send(data, bytes);
	}
}

void JcpClient::OnJcpCommand(Handler*, const string& cmd, JsonObject& root)
{
	mTickAlive = ShellTool::GetTickCount64();

	//LogV(mTag, "%s(%s)", __func__, cmd.c_str());
	KeepAlive();

	auto iter = mEntries.find(cmd);
	if (iter != mEntries.end())
	{
		auto& fun = iter->second;
		(this->*fun)(cmd, root);
	}

	SignalOnCommand(this, cmd, root);
}

void JcpClient::ParseInbox()
{
	auto obj = mProtocol.lock();
	if (obj)
	{
		obj->InputData(mInbox.data(), mInbox.length());
	}

	mInbox.clear();
}

void JcpClient::InitEntries()
{
	auto& obj = mEntries;

#define ITEM(name)	obj[#name]=(Entry)&JcpClient::On_##name
	//ITEM(profile);
}

int JcpClient::AddCommand(const string& cmd, Bundle& bundle)
{
	bundle.Set("cmd", cmd);

	DynamicJsonBuffer jBuffer;
	auto& json = jBuffer.createObject();
	for (auto& item : bundle.mItems)
	{
		json[item.first] = item.second;
	}

	sendJson(json);
	return 0;
}

void JcpClient::OnTimer(long id)
{
	if (0) {}
	else if (id == mTimer_ping)
	{
		auto tick = ShellTool::GetTickCount64();
		auto seconds = (int)((tick - mTickAlive) / 1000);
		if (seconds >= 60)
		{
			AddCommand("ping");
		}
		return;
	}
	//*/

	__super::OnTimer(id);
}

}
}
#endif
