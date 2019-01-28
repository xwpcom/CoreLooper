#pragma once
#include "iocpobject.h"
#include "iocontext.h"
#include "base/object.h"
#include "base/bytebuffer.h"
#include "looper/loop.h"
#include "looper/looper.h"
#include "net/channel.h"

#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  

namespace Bear {
namespace Core
{
namespace Net {
class TcpServer;

//XiongWanPing 2016.03.07
//TcpClient_Windows生命周期管理请见TcpServer中的说明
class CORE_EXPORT TcpClient_Windows :public IocpObject, public Channel
{
	friend class TcpServer;
	friend class TcpServer_Windows;
	friend class TcpServer_Linux;
public:
	TcpClient_Windows();
	virtual ~TcpClient_Windows();

	virtual int Connect(Bundle& info);

protected:
	virtual long GetHandle()const
	{
		return (long)mSock;
	}
	virtual int ConfigSendBuf(int bytes)
	{
		return SockTool::SetSendBuf(mSock,bytes);
	}
	virtual int ConfigRecvBuf(int bytes)
	{
		return SockTool::SetRecvBuf(mSock, bytes);
	}

	virtual int GetSendBuf(int bytes)
	{
		return -1;
	}
	virtual int GetRecvBuf(int bytes)
	{
		return -1;
	}

	virtual int ConnectHelper(std::string ip);

	virtual int DispatchIoContext(IoContext *context, DWORD bytes);
	virtual int OnRecv(IoContext *context, DWORD bytes);
	//virtual int OnRecv(LPBYTE data, int dataLen) { return -1; }
	virtual int OnSendDone(IoContext *context, DWORD bytes);

	//参照MFC::CAsyncSocket来设计
	virtual int  OnConnect(long handle, Bundle* extraInfo = nullptr);
	virtual void OnReceive();
	virtual void OnSend();
	virtual void OnClose();

	virtual void Close();
	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	void MarkEndOfSend();
	void MarkEndOfRecv();

private:
	int SendOutBox();
	void OnConnectAck();
	void ConfigCacheBox();

protected:
	HANDLE mIocp;
	SOCKET mSock;
	Bundle mBundle;
	bool   mMarkEndOfSend = false;
private:
	IoContext	mIoContextConnect;//主动连接

	IoContext	mIoContextRecv;	//接收
	IoContext	mIoContextSend;	//发送

	ByteBuffer	mInbox;			//缓存已接收到的数据
	ByteBuffer	mOutbox;		//缓存有待发送的数据

	LPFN_CONNECTEX	m_lpfnConnectEx;

	bool		mSingalClosePending;
	bool		mSignalCloseHasFired;
	std::string		mAddress;
};
}
}
}