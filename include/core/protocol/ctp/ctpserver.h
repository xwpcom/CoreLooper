#pragma once
#include "net/tcpserver.h"
#include "net/channel.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CtpHandler;

/*
XiongWanPing 2019.04.22
*/

class CORE_EXPORT CtpServer :public TcpServer2_Windows
{
	SUPER(TcpServer2_Windows)
public:
	CtpServer();

protected:
	void OnCreate();
	virtual shared_ptr<Channel> CreateChannel();
	virtual shared_ptr<CtpHandler> CreateHandler();

	virtual void OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);

};

}
}
}
}
}
