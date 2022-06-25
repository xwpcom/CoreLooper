#pragma once
#include "looper_linux.h"
#include "net/channel.h"

#ifdef __APPLE__
#include <sys/event.h>
#endif

namespace Bear {
namespace Core
{
namespace Net {
class SslFilter;
class Loop;
class CORE_EXPORT TcpClient_Linux :public Channel, public EpollProxy
{
	SUPER(Channel);
	friend class TcpServer_Linux;
public:
	TcpClient_Linux();
	virtual ~TcpClient_Linux();
	int EnableTls(bool clientMode=true);

	virtual int Connect(Bundle& info);//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	virtual long GetHandle()const
	{
		return (long)mSock;
	}

	int ConnectHelper(std::string ip);
	void OnEvent(DWORD events);

	//连接成功或失败会调用本接口
	virtual int OnConnect(long handle, Bundle* extraInfo = nullptr);

	//有数据可读时会调用本接口
	virtual void OnReceive();

	//可写时会调用本接口
	virtual void OnSend();

	//Close()会调用本接口
	virtual void OnClose();

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	void EnableListenWritable();
	void DisableListenWritable();

	void MarkEndOfRecv();
	void MarkEndOfSend();

	virtual int ConfigSendBuf(int bytes)
	{
		return SockTool::SetSendBuf(mSock, bytes);
	}
	virtual int ConfigRecvBuf(int bytes)
	{
		return SockTool::SetRecvBuf(mSock, bytes);
	}

	bool	mServerSide;//为true被动连接,为false时表示主动连接
	bool	mListenWritable;
	bool    mWaitFirstEvent;
	bool	mMarkEndOfSend = false;
	Bundle	mBundle;

	string mAddress;
	shared_ptr<SslFilter> mSslFilter;
	bool mClientMode = true;
};
}
}
}