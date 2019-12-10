#pragma once
#include "net/tcpserver.h"
#include "net/channel.h"

namespace Bear {
namespace Test {
using namespace Bear::Core;
using namespace Bear::Core::Net;


/*
	XiongWanPing 2019.04.22
	*/

	class SpeedServer :public TcpServer
	{
		SUPER(TcpServer)
	public:
		SpeedServer();

	protected:
		void OnCreate();
		virtual shared_ptr<Channel> CreateChannel();
		virtual void OnConnect(Channel* endPoint, long error, ByteBuffer* pBox, Bundle*extraInfo);

	};

}
}
