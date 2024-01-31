#include "stdafx.h"
#include "serialport_windows.h"
#include <devguid.h>
#include <setupapi.h>
#pragma comment(lib, "Setupapi.lib")

namespace Bear {
namespace Core {
using namespace FileSystem;
static const char* TAG = "SerialPort";

SerialPort_Windows::SerialPort_Windows()
{
	SetObjectName("SerialPort_Windows");
	//DV("%s,this=%p", __func__, this);
	mDataTimeout = 0;
	mOutbox.SetBufferSize(4 * 1024, 8 * 1024);
	mOutbox.PrepareBuf(4096);
}

SerialPort_Windows::~SerialPort_Windows()
{
	//DV("%s,this=%p", __func__, this);
}

void SerialPort_Windows::OnCreate()
{
	__super::OnCreate();

	ASSERT(mFile == INVALID_HANDLE_VALUE);

	USES_CONVERSION;
	auto name = A2T(mDeviceName.c_str());
	//DV("%s", mDeviceName.c_str());
	if (mDeviceName.empty())
	{
		Destroy();
		return;
	}
	
	LogV(TAG, "open %s#begin", mDeviceName.c_str());
	auto t = GetTickCount64();
	mFile = ::CreateFile(name,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);
	t = GetTickCount64()-t;
	LogV(TAG, "open %s,handler=%p,time=%lld ms", mDeviceName.c_str(),mFile,t);
	if (mFile == INVALID_HANDLE_VALUE)
	{
		if (mDeviceName != "\\\\.\\")
		{
			LogW(TAG, "fail open[%s]", mDeviceName.c_str());
		}
#ifndef _MSC_VER
		ASSERT(FALSE);
#endif
		// Display error
		Destroy();
		return;
	}

	LogV(TAG,"open serial ok:%s", mDeviceName.c_str());

/*
	//调试串口的技巧:把正常和不正常的所有配置都dump到文件，进行比较，看哪里有区别
	{
		string folder = "d:/test/good/";

		COMMCONFIG config = {0};
		DWORD bytes = sizeof(config);
		config.dwSize = sizeof(config);
		BOOL ok=GetCommConfig(mFile, &config,&bytes);
		ASSERT(ok);

		File::Dump(&config, sizeof(config), folder+"config.bin");

		DWORD mask = 0;
		ok&=GetCommMask(mFile, &mask);
		ASSERT(ok);
		File::Dump(&mask, sizeof(mask), folder + "mask.bin");

		DWORD status = 0;
		ok &= GetCommModemStatus(mFile, &status);
		ASSERT(ok);
		File::Dump(&status, sizeof(status), folder + "status.bin");

		COMMPROP prop = {0};
		ok &= GetCommProperties(mFile, &prop);
		ASSERT(ok);
		File::Dump(&prop, sizeof(prop), folder + "prop.bin");

		DCB dcb = {0};
		dcb.DCBlength = sizeof(dcb);
		ok &= GetCommState(mFile, &dcb);
		ASSERT(ok);
		File::Dump(&dcb, sizeof(dcb), folder + "dcb.bin");

		COMMTIMEOUTS timeout = {0};
		ok &= GetCommTimeouts(mFile, &timeout);
		ASSERT(ok);
		File::Dump(&timeout, sizeof(timeout), folder + "timeout.bin");

		//GetDefaultCommConfig
	}
	//*/

	{
		ByteBuffer box;
		BOOL ok = TRUE;

		if (1)
		{
			DCB dcb = { 0 };
			dcb.DCBlength = sizeof(dcb);
			ok &= GetCommState(mFile, &dcb);

			//SetCommMask(mFile, EV_CTS | EV_DSR|0xFFFF);

			ASSERT(ok);
			dcb.BaudRate = mBaudRate;// mBaudRate;// 9600; 
			dcb.ByteSize = 8;//8; 
			dcb.Parity = NOPARITY;
			dcb.StopBits = ONESTOPBIT;
			dcb.fBinary = TRUE;
			dcb.fParity = FALSE;

			ok&= SetCommState(mFile, &dcb);
			ASSERT(ok);

		}
		
		/*
		string folder = "d:/test/good/";

		{
			File::ReadFile(folder + "config.bin", box);
			COMMCONFIG *config = (COMMCONFIG *)box.GetDataPointer();
			DWORD bytes = sizeof(config);
			//config.dwSize = sizeof(config);
			ok = SetCommConfig(mFile, config, bytes);
			ASSERT(ok);
		}

		{
			File::ReadFile(folder + "mask.bin", box);
			DWORD *mask = (DWORD*)box.GetDataPointer();
			ok &= SetCommMask(mFile, *mask);
			ASSERT(ok);
		}


		//File::ReadFile(folder + "status.bin",box);
		//DWORD *status = (DWORD*)box.GetDataPointer();
		//ok &= GetCommModemStatus(mFile, status);
		//ASSERT(ok);

		//COMMPROP prop = { 0 };
		//ok &= GetCommProperties(mFile, &prop);
		//ASSERT(ok);
		//File::Dump(&prop, sizeof(prop), folder + "prop.bin");

		{
			File::ReadFile(folder + "dcb.bin", box);
			DCB *dcb = (DCB *)box.GetDataPointer();
			ok &= SetCommState(mFile, dcb);
			ASSERT(ok);
		}
		
		{
			File::ReadFile(folder + "timeout.bin", box);
			COMMTIMEOUTS *timeout = (COMMTIMEOUTS *)box.GetDataPointer();
			ok &= SetCommTimeouts(mFile, timeout);
			ASSERT(ok);
		}
		*/

		//重插id读卡器后SerialPort_Windows读不到数据,要用securecrt等其他串口工具使用一次串口，然后才能读到
		//花了很长时间才找到关键在于timeout默认数据全部为0,要设置一下才能工作正常
		//在使用其他串口设备时，没有这个问题,SerialPort_Windows能直接读到数据
		COMMTIMEOUTS timeout = {0};
		timeout.ReadIntervalTimeout = 5;
		timeout.WriteTotalTimeoutConstant = 0x1388;
		ok &= SetCommTimeouts(mFile, &timeout);
	}


	{
		IoContext& context = mIoContextSend;
		context.mSocketType = false;
		context.mType = IoContextType_Send;
		context.mSock = (SOCKET)mFile;
		context.mByteBuffer.PrepareBuf(4 * 1024);
		context.mBaseClient = dynamic_pointer_cast<IocpObject>(shared_from_this());
	}

	{
		IoContext& context = mIoContextRecv;
		context.mSocketType = false;
		context.mType = IoContextType_Recv;
		context.mSock = (SOCKET)mFile;
		context.mByteBuffer.PrepareBuf(4 * 1024);
		context.mBaseClient = dynamic_pointer_cast<IocpObject>(shared_from_this());
	}

	auto iocp = (HANDLE)Looper::CurrentLooper()->GetLooperHandle();
	HANDLE handle = CreateIoCompletionPort(mFile, iocp, (ULONG_PTR)(IocpObject*)this, 0);
	ASSERT(handle == iocp);
	mIoContextRecv.PostRecv();
}

int SerialPort_Windows::Open()
{
	return -1;
}

void SerialPort_Windows::OnDestroy()
{
	//DV("%s,this=%p", __func__,this);
	Close();
	__super::OnDestroy();
}

int SerialPort_Windows::Connect(Bundle& info)
{
	return -1;
}

void SerialPort_Windows::Close()
{
	//DV("%s", __func__);
	mClosed = true;

	ASSERT(IsMyselfThread());

	bool needFireEvent = false;
	if (mFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mFile);
		mFile = INVALID_HANDLE_VALUE;
		needFireEvent = true;
	}

	if (!mIoContextSend.mBusying)
	{
		PostDispose(mIoContextSend.mBaseClient);
	}
	if (!mIoContextRecv.mBusying)
	{
		PostDispose(mIoContextRecv.mBaseClient);
	}

	if (needFireEvent)
	{
		OnClose();
	}

	__super::Close();
}

int SerialPort_Windows::Send(LPVOID data, int dataLen)
{
	ASSERT(IsMyselfThread());

	int freeSize = mOutbox.GetFreeSize();
	int bytes = MIN(dataLen, freeSize);
	int ret = mOutbox.Write(data, bytes);

	if (ret > 0 && !mIoContextSend.mBusying)
	{
		int ack = SendOutBox();
		if (ack)
		{
			Close();
			return 0;
		}
	}

	return ret;
}

int SerialPort_Windows::Receive(LPVOID buf, int bufLen)
{
	ASSERT(buf);

	int bytes = MIN(mInbox.GetActualDataLength(), bufLen);
	if (bytes > 0)
	{
		memcpy(buf, mInbox.GetDataPointer(), bytes);
		mInbox.Eat(bytes);

		if (!mIoContextRecv.mBusying)
		{
			int freeBytes = mInbox.GetTailFreeSize();
			if (freeBytes == 0)
			{
				mInbox.MoveToHead();
			}

			freeBytes = mInbox.GetTailFreeSize();
			ASSERT(freeBytes > 0);
			if (mIoContextRecv.PostRecv(freeBytes))
			{
				Close();
			}
		}
		return bytes;
	}
	else if (mFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	return -1;
}

int SerialPort_Windows::OnConnect(long handle, Bundle* extraInfo)
{
	return -1;
}

void SerialPort_Windows::OnReceive()
{
	SignalOnReceive(this);
}

void SerialPort_Windows::OnSend()
{
	SignalOnSend(this);
}

void SerialPort_Windows::OnClose()
{
	if (mSignalCloseHasFired)
	{
		return;
	}

	bool busying = (mIoContextRecv.mBusying || mIoContextSend.mBusying);
	if (busying)
	{
		mSingalClosePending = true;
		return;
	}

	mSingalClosePending = false;
	mSignalCloseHasFired = true;
	SignalOnClose(this);
}

int SerialPort_Windows::DispatchIoContext(IoContext *context, DWORD bytes)
{
	auto objThis = shared_from_this();//确保在DispatchIoContext执行期间不被删除

	switch (context->mType)
	{
	case IoContextType_Recv:
	{
		int ret = OnRecv(context, bytes);
		break;
	}
	case IoContextType_Send:
	{
		OnSendDone(context, bytes);
		break;
	}
	default:
	{
		ASSERT(FALSE);
		break;
	}
	}

	if (mSingalClosePending)
	{
		OnClose();
	}

	return 0;
}

int SerialPort_Windows::OnRecv(IoContext *context, DWORD bytes)
{
	ASSERT(context == &this->mIoContextRecv);

	context->mBusying = false;
	bool repost = false;
	if (bytes > 0)
	{
		UpdateRecvTick();

		int ret = mInbox.Write((LPBYTE)context->mByteBuffer.GetDataPointer(), bytes);
		if (ret != bytes)
		{
			ASSERT(FALSE);//todo
		}
	}
	else if (bytes == 0)
	{
		auto error = GetLastError();
		{
			static int idx = -1;
			++idx;
			//DV("[%04d].error=%d", idx,error);
		}
		if (error != ERROR_SUCCESS && error != ERROR_IO_PENDING && error!= WAIT_TIMEOUT)
		{
			Close();
		}
		
	}

	OnReceive();

	if (!mClosed && !context->mBusying)
	{
		context->PostRecv();
	}

	if (!context->mBusying)
	{
		//DV("no recv serial port any more");
		PostDispose(context->mBaseClient);
	}

	return 0;
}

int SerialPort_Windows::OnSendDone(IoContext *context, DWORD bytes)
{
	context->mBusying = false;
	context->mByteBuffer.clear();

	UpdateSendTick();

	if (mFile == INVALID_HANDLE_VALUE)
	{
		context->mSock = INVALID_SOCKET;
		PostDispose(context->mBaseClient);
		return 0;
	}
	else
	{
		bool full = (mOutbox.GetFreeSize() == 0);
		if (!full)
		{
			OnSend();
		}

		if (!mOutbox.IsEmpty() && !mIoContextSend.mBusying)
		{
			int ret = SendOutBox();
			if (ret)
			{
			return -1;
			}
		}
	}

	return 0;
}

int SerialPort_Windows::SendOutBox()
{
	if (mFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	ASSERT(!mIoContextSend.mBusying);
	ASSERT(mIoContextSend.mByteBuffer.IsEmpty());
	ASSERT(!mOutbox.IsEmpty());

	int ret = mIoContextSend.mByteBuffer.Write(mOutbox.GetDataPointer(), mOutbox.GetActualDataLength());
	if (ret > 0)
	{
		int ack = mIoContextSend.PostSend();
		if (ack)
		{
			Close();
			return -1;
		}

		mOutbox.Eat(ret);
		return 0;
	}

	return -1;
}

void SerialPort_Windows::GetDevices(vector<tagSerialPort>& items)
{
	vector<string> friendlyNames;
	{
		/*
		SetupDiEnumDeviceInfo返回的字符串如下:
		Printer Port (LPT1)
		Communications Port (COM1)
		Prolific USB-to-Serial Comm Port (COM9)
		Prolific USB-to-Serial Comm Port (COM10)
		Prolific USB-to-Serial Comm Port (COM4)
		返回的设备可能不存在
		*/
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, 0);

		if (hDevInfo)
		{
			SP_DEVINFO_DATA SpDevInfo = { sizeof(SP_DEVINFO_DATA) };
			for (DWORD iDevIndex = 0; SetupDiEnumDeviceInfo(hDevInfo, iDevIndex, &SpDevInfo); iDevIndex++)
			{
				char szName[512] = { 0 };
				if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &SpDevInfo, SPDRP_FRIENDLYNAME,
					NULL, (PBYTE)szName, sizeof(szName), NULL))
				{
					//DV("%s", szName);
					friendlyNames.push_back(szName);
				}
			}

			SetupDiDestroyDeviceInfoList(hDevInfo);
		}
	}

	HKEY hKey;
	auto ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Hardware\\DeviceMap\\SerialComm"),
		NULL, KEY_READ, &hKey);
	if (ERROR_SUCCESS != ret)
	{
		LogW(TAG,"fail enum serial port,ret=%d", ret);
		return;
	}

	int   i = 0;
	TCHAR  RegKeyName[128], SerialPortName[128];
	DWORD  dwLong, dwSize;
	while (TRUE)
	{
		dwLong = dwSize = sizeof(RegKeyName);
		if (ERROR_NO_MORE_ITEMS == ::RegEnumValue(hKey, i, RegKeyName, &dwLong, NULL, NULL, (PUCHAR)SerialPortName, &dwSize))//枚举串口   
		{
			break;
		}
		//if (!memcmp(RegKeyName, "\\Device\\", 8))//过滤虚拟串口
		{
			//LogW(TAG,"%s", SerialPortName);   //SerialPortName就是串口名字
			USES_CONVERSION;

			tagSerialPort item;
			item.com = T2A(SerialPortName);
			{
				for (auto iter = friendlyNames.begin(); iter != friendlyNames.end(); ++iter)
				{
					auto key = " (" + item.com + ")";
					if (iter->find(key) != -1)
					{
						item.name = *iter;
						StringTool::Replace(item.name, key, "");
					}
				}
			}
			if (item.name.empty())
			{
				item.name = T2A(RegKeyName);
			}

			//LogW(TAG,"%s", item.name.c_str());
			items.push_back(item);
		}
		i++;
	}

	RegCloseKey(hKey);
}

int SerialPort_Windows::SetBaudRate(int rate)
{
	mBaudRate = rate;
	if (mFile != INVALID_HANDLE_VALUE)
	{
		DCB dcb = { 0 };
		dcb.DCBlength = sizeof(dcb);
		auto ok = GetCommState(mFile, &dcb);

		//SetCommMask(mFile, EV_CTS | EV_DSR|0xFFFF);

		dcb.BaudRate = mBaudRate;// mBaudRate;// 9600; 
		dcb.ByteSize = 8;//8; 
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fBinary = TRUE;
		dcb.fParity = FALSE;

		ok &= SetCommState(mFile, &dcb);
		int x = 0;
	}

	return 0;
}



}
}
