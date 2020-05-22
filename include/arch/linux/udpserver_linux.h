#pragma once
#include "looper_linux.h"
namespace Bear {
namespace Core
{
namespace Net {

class UdpClient_Linux;

//使用UdpClientFactory可避免重载UdpServer_Linux::CreateClient()
class UdpClientFactory
{
public:
	virtual ~UdpClientFactory() {}
	virtual std::shared_ptr<UdpClient_Linux> CreateClient() = 0;
};

//XiongWanPing 2018.02.19
class UdpServer_Linux :public Handler, public EpollProxy
{
	SUPER(Handler)
public:
	UdpServer_Linux();
	int StartServer(int port);

	void SetClientFactory(std::shared_ptr<UdpClientFactory> obj)
	{
		ASSERT(IsMyselfThread());
		mClientFactory = obj;
	}

protected:
	virtual void OnEvent(DWORD events);
	virtual int  OnAccept();
	virtual int OnNewConnect(struct sockaddr_in  *servaddr, std::shared_ptr<ByteBuffer> box);
	virtual std::shared_ptr<UdpClient_Linux>CreateClient();

	std::shared_ptr<UdpClientFactory> mClientFactory;
	int mPort = 0;
	std::map<ULONGLONG, std::weak_ptr<UdpClient_Linux>> mClients;
};
}
}
}