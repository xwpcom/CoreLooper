#pragma once
#include "base/bytebuffer.h"
#include "base/object.h"
#include "iocontext.h"
namespace Bear {
namespace Core
{
namespace Net {

class Loop;
class CORE_EXPORT UdpIoContext :public IoContext, public std::enable_shared_from_this<UdpIoContext>
{
public:
	UdpIoContext();
	virtual ~UdpIoContext();

	virtual int PostRecv(int maxRecvBytes = 0);
	std::shared_ptr<IoContext> mSelfRef;
	struct sockaddr  mSockAddr;
	int	mAddrLen = 0;
};
}
}
}