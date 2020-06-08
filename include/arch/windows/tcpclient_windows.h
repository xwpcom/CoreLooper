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

#ifdef _CONFIG_OPENSSL
namespace toolkit
{
class BufferRaw;
class SSL_Box;
}
#endif

namespace Bear {
namespace Core
{
namespace Net {
class TcpServer;

#ifdef _CONFIG_OPENSSL
using namespace toolkit;
#endif

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

#ifdef _CONFIG_OPENSSL
	void EnableTls();
#endif

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

	virtual int GetOutboxCacheBytes(); 

private:
	void CheckInitTls(bool serverMode);

	int SendOutBox();
	void CheckSend();
	void OnConnectAck();
	void ConfigCacheBox();

protected:
	HANDLE mIocp = INVALID_HANDLE_VALUE;
	SOCKET mSock = INVALID_SOCKET;
	Bundle mBundle;
	bool   mMarkEndOfSend = false;
private:
	IoContext	mIoContextConnect;//主动连接

	IoContext	mIoContextRecv;	//接收
	IoContext	mIoContextSend;	//发送

	virtual ByteBuffer* GetRawInbox();
	ByteBuffer	mInbox;			//缓存已接收到的数据
	ByteBuffer	mOutbox;		//缓存有待发送的数据

	LPFN_CONNECTEX	m_lpfnConnectEx=nullptr;
	bool		mSingalClosePending = false;
	bool		mSignalCloseHasFired = false;
	string		mAddress;
	bool		mReceiveBusying=false;

#ifdef _CONFIG_OPENSSL
	struct tagTlsInfo
	{
		//用ptr是为了避免在.h中引入ssl相关header(否则会导致app也要引用ssl header)
		shared_ptr<BufferRaw> mInBuffer;
		//shared_ptr<BufferRaw> mOutBuffer;
		shared_ptr<SSL_Box>	  mSslBox;
		ByteBuffer mInboxSSL;
		ByteBuffer mOutboxSSL;
	};

	unique_ptr<tagTlsInfo> mTlsInfo;
#endif

};
}
}
}