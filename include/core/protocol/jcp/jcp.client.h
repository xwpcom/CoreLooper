#pragma once
#if defined _CONFIG_JCP
#include "core/net/simpleconnect.h"

namespace Bear {
namespace Jcp {
using namespace Bear::Core;
using namespace Bear::Core::Net;

class JcpProtocol;
/*
XiongWanPing 2022.05.15
*/
class CORE_EXPORT JcpClient :public SimpleConnect
{
	SUPER(SimpleConnect)
public:
	JcpClient();
	int AddCommand(const string& cmd)
	{
		Bundle bundle;
		return AddCommand(cmd, bundle);
	}
	int AddCommand(const string& cmd, Bundle& bundle);
	sigslot::signal3<Handler*, const string&, JsonObject&> SignalOnCommand;
	void EnableDumpCommand(bool enable = true)
	{
		mDumpCommand = enable;
	}
	void sendJson(JsonObject& json);
protected:
	void OnCreate();
	void OnConnect(Channel* endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	virtual void OnJcpCommand(Handler*, const string& cmd, JsonObject& root);
	void OnTimer(long id)override;

	virtual void ParseInbox();
	void OnJcpWrite(Handler*, LPBYTE data, int bytes);
	void OnJcpError(Handler*, const string& desc);

	weak_ptr<JcpProtocol> mProtocol;

	typedef void (JcpClient::* Entry)(const string& cmd, JsonObject& obj);
	unordered_map<string, Entry> mEntries;
	virtual void InitEntries();

	bool mDumpCommand = false;
	string mTag = "JcpClient";

	long mTimer_ping = 0;
	int mSeq = 0;
	ULONGLONG mTickAlive = 0;
};

}
}
#endif

