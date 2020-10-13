#pragma once
#include "net/tcpserver.h"
#include "net/channel.h"
#include "net/tcpclient.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

class CtpHandler2;

/*
XiongWanPing 2019.04.22
*/

#ifdef _MSC_VER
#define CTPS_BASE TcpServer2_Windows
#else
#define CTPS_BASE TcpServer_Linux
#endif

class CORE_EXPORT CtpServer2 :public CTPS_BASE
{
	SUPER(CTPS_BASE)
public:
	CtpServer2();

protected:
	void OnCreate();
	virtual shared_ptr<Channel> CreateChannel();
	virtual shared_ptr<CtpHandler2> CreateHandler();

	virtual void OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);

};

}
}
}
}
}
