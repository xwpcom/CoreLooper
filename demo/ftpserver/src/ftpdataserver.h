#pragma once
#include "net/tcpserver.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class FtpDataHandler;

enum eFtpDataMode
{
	eFtpDataMode_None,
	eFtpDataMode_In,
	eFtpDataMode_Out,
};

//XiongWanPing 2018.04.24
class FtpDataServer :public TcpServer
{
	SUPER(TcpServer)
public:
	FtpDataServer();
	~FtpDataServer();

	shared_ptr<FtpDataHandler> GetDataHandler()
	{
		auto obj = mDataHandler.lock();
		return obj;
	}

	void SetMode(eFtpDataMode mode);

	sigslot::signal1<shared_ptr<FtpDataHandler>> SignalOnConnect;
protected:
	shared_ptr<Channel> CreateChannel();
	void OnConnect(Channel* endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo);
	void HandleConnect(shared_ptr<Channel> endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);

	weak_ptr<FtpDataHandler> mDataHandler;

	weak_ptr<Channel>	mChannelCache;
	eFtpDataMode mMode = eFtpDataMode_None;
};

}
}
}
}