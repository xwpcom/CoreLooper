#pragma once
#include "protocol/ctp/CommonTextProtocol.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CORE_EXPORT CtpHandler :public Handler, public CommonTextProtocolCB
{
	SUPER(Handler);
	friend class CtpServer;

public:
	CtpHandler();
	~CtpHandler();

	shared_ptr<Channel> mDataEndPoint;
protected:
	void OnCreate();
	void OnTimer(long id);

	virtual void AddMessage(const string& cmd)
	{
		Bundle d;
		AddMessage(cmd, d);
	}
	virtual void AddMessage(const string& cmd, const Bundle& bundle);

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

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	void CheckSend();

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;

	ByteBuffer	mOutbox;
	CommonTextProtocol* mProtocol = nullptr;

	long mTimer_CheckAlive = 0;
};

}
}
}
}
}
