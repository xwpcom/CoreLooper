#pragma once
#include "looper/handler.h"
#include "looper_linux.h"
#include "net/tcpclient.h"

#ifdef __APPLE__
#include <sys/event.h>
#endif

namespace Bear {
namespace Core
{
namespace Net {
class TcpClient_Linux;
enum
{
	BM_CLIENT_CLOSE,
};

class CORE_EXPORT TcpServer_Linux :public Handler, public EpollProxy
{
	SUPER(Handler)
public:
	TcpServer_Linux();
	virtual ~TcpServer_Linux();

	int StartServer(int port);
	void Stop();
	int GetPort()const
	{
		return mPort;
	}

protected:
	virtual std::shared_ptr<Channel> CreateChannel();

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void OnEvent(DWORD events);
	virtual int OnAccept();
protected:
	int mPort;
};
}
}
}