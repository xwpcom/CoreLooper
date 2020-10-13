#pragma once
#include "net/simpleconnect.h"
#include "protocol/ctp/CommonTextProtocol2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CORE_EXPORT CtpClient2 :public SimpleConnect, public CommonTextProtocolCB2
{
	SUPER(SimpleConnect)
public:
	CtpClient2();
	~CtpClient2();


	virtual void AddMessage(const string& cmd)
	{
		Bundle d;
		AddMessage(cmd, d);
	}
	virtual void AddMessage(const string& cmd, const Bundle& bundle);

protected:
	void OnCreate();
	void OnConnect(Channel* endPoint, long error, ByteBuffer*, Bundle* extraInfo)override;
	void OnTimer(long id);

	//CommonTextProtocolCB#begin
	virtual void OnCommand(CommonTextProtocol2* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& inputBody);

	//协议等出错时调用本接口
	//正常情况下不会触发,仅用于开发调试
	virtual void OnError(CommonTextProtocol2* obj, int error, const string& desc)override;

	//有数据要发给对方时，会调用本接口
	virtual void Output(CommonTextProtocol2* obj, const ByteBuffer& data);
	//CommonTextProtocolCB#end

	void ParseInbox();
	CommonTextProtocol2* mProtocol = nullptr;
	long mTimer_CheckAlive = 0;
	ULONGLONG mTickAlive = 0;

	void UpdateTickAlive()
	{
		mTickAlive = ShellTool::GetTickCount64();
	}

};

}
}
}
}
}

