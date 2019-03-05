#pragma once

#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  

#include "looper/loop.h"
#include "arch/windows/iocpobject.h"
#include "arch/windows/iocontext.h"
#include "base/object.h"
//XiongWanPing 2016.03.07
/*
网络框架生命周期管理
参与方:Looper,TcpServer_Windows和TcpClient
以TcpServer_Windows为中心,当TcpServer_Windows失效时可安全退出Looper

Looper生命周期管理
.Looper保持对TcpServer_Windows的弱引用,在Looper::CanQuitLooperNow()中如果TcpServer_Windows已失效，则可以安全退出looper
 可保证在TcpServer_Windows有效时，不会退出主循环

TcpServer_Windows生命周期管理
.TcpServer_Windows.PostAccept,每个outstanding IoContext.mBaseTcpServer_Windows引用一次,这样可保证有未完结的accept调用时，不会删除TcpServer_Windows
.TcpClient.mBaseTcpServer_Windows,可保证TcpClient业务进行时，不会删除TcpServer_Windows

何时能删除TcpServer_Windows
.调用TcpServer_Windows::Stop()来关闭listen socket,保证不再接受新的连接，并且会完结所有的outstanding PostAccept
.TcpServer_Windows::Stop()会主动断开所有已存在的连接

TcpClient生命周期管理
.每个outstanding的IoContext.mBaseClient引用一次
.在析构TcpClient时会采用安全的方式通知清除TcpServer_Windows.mClients中的弱引用,并可最终删除TcpServer_Windows
//*/
namespace Bear {
namespace Core
{
namespace Net {
class TcpClient;
class CORE_EXPORT TcpServer_Windows :public IocpObject, public Handler
{
	SUPER(Handler);
	friend class TcpClient;
public:
	TcpServer_Windows();
	virtual ~TcpServer_Windows();

	virtual int StartServer(int port);
	virtual void Stop();

	int GetPort()const
	{
		return mPort;
	}

protected:
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual std::shared_ptr<Channel> CreateChannel();

	int PostAccept(IoContext *ioContext);
	virtual int DispatchIoContext(IoContext *context, DWORD bytes);

	virtual int OnAccept(SOCKET s);

private:
	HANDLE mIocp;
	SOCKET mSock;
	int mPort;

	LPFN_ACCEPTEX mAcceptEx=nullptr;
	LPFN_GETACCEPTEXSOCKADDRS mGetAcceptExSockAddrs = nullptr;
};
}
}
}