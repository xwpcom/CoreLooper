#pragma once
#include "net/channel.h"
namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2018.02.21
/* 项目中还没用到udp,没真正实现 */
class UdpClient_Windows :public Channel
{
public:
	UdpClient_Windows();
	~UdpClient_Windows();

	int AttachSocket(SOCKET s);

protected:
	virtual int Connect(Bundle& info);
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	void OnCreate();
	void OnTimer(long timerId);

	virtual int OnConnect(long handle, Bundle* extraInfo);

	virtual void OnReceive();

	virtual void OnSend();
	virtual void OnClose();

	SOCKET mSock = INVALID_SOCKET;
};
}
}
}