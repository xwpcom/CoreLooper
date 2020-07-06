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

	//�յ��Է��Ļظ�ʱ���ñ��ӿ�
	virtual void OnCommandAck(CommonTextProtocol* obj, const string& cmd, const Bundle& reqBundle, const Bundle& ackBundle, const ByteBuffer& ackBody);

	//�յ��Է���֪ͨʱ���ñ��ӿ�
	virtual void OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body);

	//Э��ȳ���ʱ���ñ��ӿ�
	//��������²��ᴥ��,�����ڿ�������
	virtual void OnError(CommonTextProtocol* obj, int error, const string& desc);

	//������Ҫ�����Է�ʱ������ñ��ӿ�
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

