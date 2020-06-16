#pragma once

#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  

#include "looper/loop.h"
#include "arch/windows/iocpobject.h"
#include "arch/windows/iocontext.h"
#include "base/object.h"
//XiongWanPing 2020.06.15
namespace Bear {
namespace Core
{
namespace Net {

class CORE_EXPORT TcpListener_Windows:public Handler, public IocpObject
{
	SUPER(Handler);
public:
	TcpListener_Windows();
	~TcpListener_Windows();

	int StartListener(int port);
	void StopListener();

	sigslot::signal2<Handler*, SOCKET> SignalAccept;
protected:
	void OnDestroy();

	int PostAccept(IoContext* ioContext);
	int DispatchIoContext(IoContext* context, DWORD bytes);
	int OnAccept(SOCKET s);

	HANDLE mIocp= INVALID_HANDLE_VALUE;
	SOCKET mSock= INVALID_SOCKET;
	int mPort=0;

	LPFN_ACCEPTEX mAcceptEx = nullptr;
	LPFN_GETACCEPTEXSOCKADDRS mGetAcceptExSockAddrs = nullptr;

};

}
}
}
