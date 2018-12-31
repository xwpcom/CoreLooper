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

class UdpClient_Linux :public Channel, public EpollProxy
{
	SUPER(Channel)
		friend class UdpServer_Linux;
public:
	UdpClient_Linux();
	~UdpClient_Linux();
	int AttachSocket(SOCKET s);

	virtual int Connect(Bundle& info);//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
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

	bool	mServerSide = true;//为true被动连接,为false时表示主动连接
	bool	mListenWritable = false;
	bool    mWaitFirstEvent = true;
	std::string mAddress;

	void AddPendingData(std::shared_ptr<ByteBuffer> box)
	{
		mPendingData.push_back(box);
	}
	std::list<std::shared_ptr<ByteBuffer>> mPendingData;
};
}
}
}