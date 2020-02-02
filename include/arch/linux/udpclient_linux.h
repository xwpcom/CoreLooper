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

	virtual int Connect(Bundle& info);
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	int ConnectHelper(std::string ip);
	void OnEvent(DWORD events);

	virtual int OnConnect(long handle, Bundle* extraInfo = nullptr);

	virtual void OnReceive();
	virtual void OnSend();

	virtual void OnClose();

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	void EnableListenWritable();
	void DisableListenWritable();

	bool	mServerSide = true;
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