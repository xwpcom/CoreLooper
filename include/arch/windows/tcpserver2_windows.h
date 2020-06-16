#pragma once

//XiongWanPing 2020.06.16
//TcpServer2_Windows是TcpServer_Windows改进版，支持侦听多个port

namespace Bear {
namespace Core
{
namespace Net {
class TcpClient;
class CORE_EXPORT TcpServer2_Windows :public Handler
{
	SUPER(Handler);
	friend class TcpClient;
public:
	TcpServer2_Windows();
	virtual ~TcpServer2_Windows();
	
	int StartServer(int port)
	{
		return StartListenPort(port);
	}

	int StartListenPort(int port);

#ifdef _CONFIG_OPENSSL
	static int InitSSL(const string& filePath);
#endif
protected:
	virtual std::shared_ptr<Channel> CreateChannel();

	virtual void OnAccept(Handler*,SOCKET s);

protected:
};
}
}
}