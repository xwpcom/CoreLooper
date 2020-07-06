#pragma once
#include "net/simpleconnect.h"
#include "protocol/ctp/CommonTextProtocol.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CORE_EXPORT CtpClient :public SimpleConnect, public CommonTextProtocolCB
{
	SUPER(SimpleConnect)
public:
	CtpClient();
	~CtpClient();

protected:
	void OnCreate();
	void OnConnect(Channel* endPoint, long error, ByteBuffer*, Bundle* extraInfo);
	void OnTimer(long id);

	//CommonTextProtocolCB#begin
	virtual void OnCommand(CommonTextProtocol* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody, Bundle& ackBundle, ByteBuffer& ackBody);

	//收到对方的回复时调用本接口
	virtual void OnCommandAck(CommonTextProtocol* obj, const string& cmd, const Bundle& reqBundle, const Bundle& ackBundle, const ByteBuffer& ackBody);

	//收到对方的通知时调用本接口
	virtual void OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body);

	//协议等出错时调用本接口
	//正常情况下不会触发,仅用于开发调试
	virtual void OnError(CommonTextProtocol* obj, int error, const string& desc);

	//有数据要发给对方时，会调用本接口
	virtual void Output(CommonTextProtocol* obj, const ByteBuffer& data);
	//CommonTextProtocolCB#end

	void ParseInbox();
	CommonTextProtocol* mProtocol = nullptr;
};

}
}
}
}
}

