#pragma once
#include "net/tcpserver.h"
#include "file/virtualfolder.h"
#include "ftpconfig.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {

//XiongWanPing 2018.04.20
class FtpServer :public TcpServer
{
	SUPER(TcpServer)

public:
	FtpServer();

	void SetConfig(shared_ptr<tagFtpServerConfig> config)
	{
		mConfig = config;
	}

protected:
	void OnCreate();

	shared_ptr<Channel> CreateChannel();
	void OnConnect(Channel* endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo);
	void HandleConnect(shared_ptr<Channel> endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);

	shared_ptr<tagFtpServerConfig> mConfig;
};
}
}
}
}