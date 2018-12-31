#pragma once
#include "IoContext.h"
#include "net/channel.h"
#include "iocpobject.h"
namespace Bear {
namespace Core {
using namespace Net;

struct tagSerialPort
{
	string name;
	string com;
};

//XiongWanPing 2018.03.12
class CORE_EXPORT SerialPort_Windows :public IocpObject, public Channel
{
	SUPER(Channel)
public:
	SerialPort_Windows();
	~SerialPort_Windows();
	static void GetDevices(vector<tagSerialPort>& items);
	int GetBaudRate()const
	{
		return mBaudRate;
	}

	void SetBaudRate(int rate)
	{
		mBaudRate = rate;
	}

	void SetDeviceName(string name)
	{
		mDeviceName = name;
	}

	string GetDeviceName()const
	{
		return mDeviceName;
	}

	virtual int Connect(Bundle& info);//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();
	virtual int Send(LPVOID data, int dataLen);
	virtual int Receive(LPVOID buf, int bufLen);

protected:
	void OnCreate();
	void OnDestroy();

	virtual int DispatchIoContext(IoContext *context, DWORD bytes);
	virtual void OnCustomIocpMessage(UINT msg, LPVOID info);

	virtual int OnConnect(long handle, Bundle* extraInfo);
	virtual void OnReceive();
	virtual void OnSend();
	virtual void OnClose();

	int OnRecv(IoContext *context, DWORD bytes);
	int OnSendDone(IoContext *context, DWORD bytes);
	int SendOutBox();

	int mBaudRate = 115200;
	string mDeviceName;// CString com = "\\\\.\\COM48";
	HANDLE mFile = INVALID_HANDLE_VALUE;

	IoContext	mIoContextRecv;	//接收
	IoContext	mIoContextSend;	//发送

	ByteBuffer	mInbox;			//缓存已接收到的数据
	ByteBuffer	mOutbox;		//缓存有待发送的数据

	bool		mSingalClosePending = false;
	bool		mSignalCloseHasFired = false;
	bool mClosed = false;
};

}
}
