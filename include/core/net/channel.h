#pragma once

#include "core/base/bundleex.h"
#include "core/base/sigslot.h"
#include "core/looper/handler.h"
namespace Bear {
namespace Core
{
namespace Net {
/*
XiongWanPing 2016.03.17
仿照CAsyncSocket,提供数据收发抽象接口
https://msdn.microsoft.com/en-us/library/3d46645f.aspx
数据传输框架设计要点
.要兼容epoll
 采用统一的接口支持windows和linux平台
.要兼容p2p
 不要局限于socket api,要考虑兼容p2p api接口,提供统一抽象数据传输层框架
*/

//数据端口基类
class CORE_EXPORT Channel :public Handler
{
	SUPER(Handler)
public:
	Channel();
	virtual ~Channel();

	virtual int Connect(Bundle& info) = 0;//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual void EnableVerbose()
	{
		mVerbose = true;
	}
	virtual void DisableVerbose()
	{
		mVerbose = false;
	}

	virtual int Send(LPVOID data, int dataLen) = 0;
	virtual int Receive(LPVOID buf, int bufLen) = 0;

	virtual void MarkEndOfSend() {}
	virtual void MarkEndOfRecv() {}
	virtual const std::string& GetLocalDesc()const
	{
		return mLocalDesc;
	}

	virtual int ConfigSendBuf(int bytes)
	{
		bytes = bytes;
		return -1;
	}
	virtual int ConfigRecvBuf(int bytes)
	{
		bytes = bytes;
		return -1;
	}

	virtual int GetSendBuf(int bytes)
	{
		bytes = bytes;
		return -1;
	}
	virtual int GetRecvBuf(int bytes)
	{
		bytes = bytes;
		return -1;
	}

	virtual const std::string& GetPeerDesc()const
	{
		return mPeerDesc;
	}

	virtual long GetHandle()const
	{
		return -1;
	}

	//连接成功或失败会调用本接口
	virtual int OnConnect(long handle, Bundle* extraInfo) = 0;
	virtual int GetOutboxCacheBytes()
	{
		return 0;
	}
protected:
	void OnCreate();
	void OnTimer(long timerId);
	void OnDestroy();


	//有数据可读时会调用本接口
	virtual void OnReceive() = 0;

	//可写时会调用本接口
	virtual void OnSend() = 0;

	//Close()会调用本接口
	virtual void OnClose() = 0;

	virtual void Close_Impl();

public:
	sigslot::signal4<Channel*, long, ByteBuffer *, Bundle*>	SignalOnConnect;
	sigslot::signal1<Channel*>								SignalOnReceive;
	sigslot::signal1<Channel*>								SignalOnSend;
	sigslot::signal1<Channel*>								SignalOnClose;

protected:
	void UpdateRecvTick();
	void UpdateSendTick();
	LRESULT HandleClose(WPARAM wp,LPARAM lp);

	ULONGLONG	mTickLastRecv;
	ULONGLONG	mTickLastSend;
	UINT		mDataTimeout;
	struct tm	mTimeCreate;
	std::string		mLocalDesc;
	std::string		mPeerDesc;
	long		mTimerCheckAlive = 0;
	bool mVerbose = true;
	const UINT mMessageClose= BindMessage(&Channel::HandleClose);
};
}
}
}