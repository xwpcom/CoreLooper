#pragma once
#ifdef _MSC_VER
#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  
#endif

#include "looper/loop.h"
#include "arch/windows/iocpobject.h"
#include "base/object.h"

namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2018.02.18
class CORE_EXPORT UdpServer_Windows :public IocpObject, public Handler
{
	SUPER(Handler)
public:
	UdpServer_Windows();
	virtual ~UdpServer_Windows();

	virtual int StartServer(int port);
	virtual void Stop();

	int GetPort()const
	{
		return mPort;
	}

	virtual void OnRecv(LPBYTE data, int dataBytes, const sockaddr& addr);

protected:
	virtual LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual int DispatchIoContext(IoContext *context, DWORD bytes);

protected:
	HANDLE mIocp = INVALID_HANDLE_VALUE;
	SOCKET mSock = INVALID_SOCKET;
	int mPort = 0;
};
}
}
}