#include "stdafx.h"
#include "udpclient_windows.h"
namespace Bear {
namespace Core
{
namespace Net {

UdpClient_Windows::UdpClient_Windows()
{

}

UdpClient_Windows::~UdpClient_Windows()
{

}

int UdpClient_Windows::AttachSocket(SOCKET s)
{
	ASSERT(mSock == -1);
	mSock = s;
	return 0;
}

int UdpClient_Windows::Connect(Bundle& info)
{
	DV("%s", __func__);
	return -1;
}

void UdpClient_Windows::Close()
{
	DV("%s", __func__);
}

int UdpClient_Windows::Send(LPVOID data, int dataLen)
{
	DV("%s", __func__);
	return -1;
}

int UdpClient_Windows::Receive(LPVOID buf, int bufLen)
{
	DV("%s", __func__);
	return -1;
}

void UdpClient_Windows::OnCreate()
{
	DV("%s", __func__);
}

void UdpClient_Windows::OnTimer(long timerId)
{
	DV("%s", __func__);
}

int UdpClient_Windows::OnConnect(long handle, Bundle* extraInfo)
{
	DV("%s", __func__);
	return -1;
}

void UdpClient_Windows::OnReceive()
{
	DV("%s", __func__);
}

void UdpClient_Windows::OnSend()
{
	DV("%s", __func__);
}

//Close()会调用本接口
void UdpClient_Windows::OnClose()
{

}

}
}
}