#pragma once
#include "arch/linux/looper_linux.h"
#include "net/channel.h"
namespace Bear {
namespace Core {

//XiongWanPing 2017.02.07
class CORE_EXPORT SerialPort_Linux :public Bear::Core::Net::Channel, public EpollProxy
{
	SUPER(Channel);
public:
	SerialPort_Linux();
	virtual ~SerialPort_Linux();

	sigslot::signal2<Handler*,int>	SignalSerialOpenAck;

	virtual int Connect(Bundle& info);//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

	int GetBaudRate()const
	{
		return mBaudRate;
	}

	void SetBaudRate(int rate)
	{
		mBaudRate = rate;
	}

	void SetDeviceName(string  name)
	{
		mDeviceName = name;
	}

	string  GetDeviceName()const
	{
		return mDeviceName;
	}

protected:
	void OnCreate();
	void OnEvent(DWORD events);

	virtual int OnConnect(long handle, Bundle* extraInfo);
	virtual void OnReceive();
	virtual void OnSend();
	virtual void OnClose();

	static int SetComSpeed(int fd, unsigned int baud_rate);

	string  mDeviceName;//dev/ttyS0...

	int mHandle = -1;
	int mBaudRate = 9600;
};
}
}
