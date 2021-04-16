#pragma once
#include "websockethandler.h"
#include "protocol/ctp/CommonTextProtocol.h"

namespace Bear {
namespace Core {
namespace Net {
using namespace Protocol::CTP;

namespace Http {

class HTTP_EXPORT  WSHandler :public Handler, public CommonTextProtocolCB
{
	SUPER(Handler);
public:
	WSHandler();
	~WSHandler();

	sigslot::signal2<Handler*, ByteBuffer&> SignalSend;
	sigslot::signal1<Handler*> SignalDestroy;

	virtual void OnWebSocketRecv(Handler*, LPBYTE, int);
	virtual void OnWebSocketClosed(Handler*);
protected:
	void OnCreate();
	void OnDestroy();
	void OnTimer(long id);

	//CommonTextProtocolCB#begin
	virtual void OnCommand(CommonTextProtocol* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody, Bundle& ackBundle, ByteBuffer& ackBody);

	//收到对方的通知时调用本接口
	virtual void OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body);

	//协议等出错时调用本接口
	//正常情况下不会触发,仅用于开发调试
	virtual void OnError(CommonTextProtocol* obj, int error, const string& desc);

	//有数据要发给对方时，会调用本接口
	virtual void Output(CommonTextProtocol* obj, const ByteBuffer& data);
	//CommonTextProtocolCB#end

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;
	long mTimer_CheckAlive = 0;
	long mTimeoutSeconds = 5 * 60;

	CommonTextProtocol* mProtocol = nullptr;
};

}
}
}
}
