#pragma once
#include "net/channel.h"
namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2018.02.21
class UdpClient_Windows :public Channel
{
public:
	UdpClient_Windows();
	~UdpClient_Windows();

	int AttachSocket(SOCKET s);

protected:
	virtual int Connect(Bundle& info);//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	void OnCreate();
	void OnTimer(long timerId);

	//连接成功或失败会调用本接口
	virtual int OnConnect(long handle, Bundle* extraInfo);

	//有数据可读时会调用本接口
	virtual void OnReceive();

	//可写时会调用本接口
	virtual void OnSend();

	//Close()会调用本接口
	virtual void OnClose();

	SOCKET mSock = INVALID_SOCKET;
};
}
}
}