#pragma once
extern "C"
{
#include "protocol/sctp/cbundle.h"
#include "protocol/sctp/csctp.h"
}

#include "net/channel.h"
#include "core/base/bytetool.h"
#include "net/simpleconnect.h"

struct tagBundle;

namespace SCTP {
using namespace Bear::Core;
using namespace Bear::Core::Net;
class Sctp;

/*
XiongWanPing 2021.06.05

*/
class CORE_EXPORT SctpClient :public SimpleConnect
{
	SUPER(SimpleConnect)
public:
	SctpClient();
	int AddCommand(const string& cmd)
	{
		Bundle bundle;
		return AddCommand(cmd, bundle);
	}
	int AddCommand(const string& cmd, Bundle& bundle);
	sigslot::signal3<Handler*, const string&, JsonObject&> SignalOnCommand;
	void EnableDumpCommand(bool enable=true)
	{
		mDumpCommand = enable;
	}
protected:
	void OnCreate();

	virtual void ParseInbox();
	virtual void OnRecvCommand(Sctp* obj, const char* szCommand, tagBundle* params);

	weak_ptr<Sctp> mProtocol;

	typedef void (SctpClient::* Entry)(const string& cmd, JsonObject& obj);
	unordered_map<string, Entry> mEntries;
	virtual void InitEntries();

	bool mDumpCommand = false;
};

}
