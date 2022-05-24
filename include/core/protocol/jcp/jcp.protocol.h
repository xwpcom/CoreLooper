#pragma once

namespace Bear {
namespace Jcp {
using namespace Bear::Core;

class CORE_EXPORT JcpProtocol :public Handler
{
	SUPER(Handler);
public:
	JcpProtocol();
	sigslot::signal3<Handler*, const string&, JsonObject&> SignalJcpCommand;
	sigslot::signal3<Handler*, LPBYTE, int> SignalJcpWrite;
	sigslot::signal2<Handler*, const string&> SignalJcpError;
	void InputData(LPBYTE data, int bytes);
	void sendJson(JsonObject& json);
protected:
	void OnCreate();
	string mTag = "JcpProtocol";
	ByteBuffer mInbox;
};

}
}