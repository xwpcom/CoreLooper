#pragma once
#include "protocol/ctp/CommonTextProtocol2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CORE_EXPORT CtpHandler2 :public Handler, public CommonTextProtocolCB2
{
	SUPER(Handler);
	friend class CtpServer2;

public:
	CtpHandler2();
	~CtpHandler2();

	shared_ptr<Channel> mDataEndPoint;
	virtual void AddCommand(const string& cmd)
	{
		Bundle d;
		AddCommand(cmd, d);
	}
	virtual void AddCommand(const string& cmd, const Bundle& bundle);
protected:
	void OnCreate();
	void OnTimer(long id);

	//CommonTextProtocolCB2#begin
	virtual void OnCommand(CommonTextProtocol2* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody);

	//协议等出错时调用本接口
	//正常情况下不会触发,仅用于开发调试
	virtual void OnError(CommonTextProtocol2* obj, int error, const string& desc);

	//有数据要发给对方时，会调用本接口
	virtual void Output(CommonTextProtocol2* obj, const ByteBuffer& data);
	//CommonTextProtocolCB2#end

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);
	void CheckSend();

	void UpdateTickAlive();
	ULONGLONG mTickAlive = 0;

	ByteBuffer	mOutbox;
	CommonTextProtocol2* mProtocol = nullptr;

	long mTimer_CheckAlive = 0;
};

}
}
}
}
}
