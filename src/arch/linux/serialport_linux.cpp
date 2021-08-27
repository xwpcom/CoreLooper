#include "stdafx.h"
#include "serialport_linux.h"

#if defined _MSC_VER && !defined O_NOCTTY
#define O_NOCTTY 0
#endif

#ifndef _MSC_VER
#define _open open
#define _read read
#define _write write

#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX终端控制定义*/

#endif
namespace Bear {
namespace Core {
static const char *TAG="SerialPort_Linux";
SerialPort_Linux::SerialPort_Linux()
{
	SetObjectName("SerialPort");
	mHandle = -1;
	mDataTimeout = 0;
}

SerialPort_Linux::~SerialPort_Linux()
{

}

int SerialPort_Linux::Connect(Bundle& info)
{
	return -1;
}

void SerialPort_Linux::Close()
{
	if (mHandle >= 0)
	{
		_close(mHandle);
		mHandle = -1;
	}
}

int SerialPort_Linux::Send(LPVOID data, int dataLen)
{
	int ret = _write(mHandle, data, dataLen);
	//static int idx = -1;
	//++idx;
	if (ret != dataLen)
	{
		LogW(GetObjectName(),"fail write,len=%d,ret=%d,error=%d(%s)", dataLen,ret,errno,strerror(errno));
	}
	return ret;
}

int SerialPort_Linux::Receive(LPVOID buf, int bufLen)
{
	int ret = _read(mHandle, buf, bufLen);
	if (ret > 0)
	{
		UpdateRecvTick();
	}
	else
	{
		int err = errno;
		//LogW(TAG, "fail read#1,error=%d(%s)",err,strerror(err));
	}
	return ret;
}

void SerialPort_Linux::OnEvent(DWORD events)
{
	auto objThis = shared_from_this();//确保在OnEvent执行期间不被删除

	//LogV(TAG,"SerialPort_Linux::events=0x%02x", events);

	if (events & EPOLLIN)
	{
		OnReceive();
	}
}

int SerialPort_Linux::OnConnect(long handle, Bundle* extraInfo)
{
	return -1;
}

void SerialPort_Linux::OnReceive()
{
	//LogV("%s",__func__);
	SignalOnReceive(this);
}

void SerialPort_Linux::OnSend()
{

}

void SerialPort_Linux::OnClose()
{
	if (mHandle != -1)
	{
		unsigned long handle = (unsigned long)(LONGLONG)GetCurrentLooper()->GetLooperHandle();
		int ret = -1;
#ifdef __APPLE__
		//感觉没有这个需求,没人会在apple电脑上用串口
		ASSERT(FALSE);
#else
		struct epoll_event evt = { 0 };
		ret = epoll_ctl((int)handle, EPOLL_CTL_DEL, mHandle, &evt);//remove all events
#endif
	}
}

int SerialPort_Linux::SetComSpeed(int fd, unsigned int baud_rate)
{
	LogV(TAG,"SetComSpeed(fd=%d,rate=%d)", fd, baud_rate);

#ifndef _MSC_VER
	int databits = 8;
	int parity = 0;
	int stopbits = 1;

	unsigned int i, index = 0;
	struct termios options;

	unsigned int speed_arr[] = { B0, B50,B75, B110, B134,B150 ,B200, B300, B600, B1200, B1800, B2400,
		B4800,B9600, B19200,B38400, B57600,B115200,B230400 };/*baud rate table*/

	unsigned int name_arr[] = { 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600,
		19200, 38400, 57600, 115200, 230400 };

	for (i = 0; i < sizeof(name_arr) / sizeof(int); i++)
	{
		if (name_arr[i] == baud_rate)
			break;
	}

	if (i >= sizeof(name_arr) / sizeof(int))
	{
		ASSERT(FALSE);
		return -1;
	}

	//if(fd == rs232_fd)
	//	index = 0;
	//else
	index = 1;

	memset(&options, 0x00, sizeof(options));
	if (0 != tcgetattr(fd, &options))
	{
		ASSERT(FALSE);
		return -1;
	}

	tcflush(fd, TCIOFLUSH); //清空缓存
	cfsetispeed(&options, speed_arr[i]);
	cfsetospeed(&options, speed_arr[i]);

	/*
	CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能
	CREAD--使能接收标志
	CSIZE--字符长度掩码。取值为 CS5, CS6, CS7, 或 CS8
	*/
	options.c_cflag |= (CLOCAL | CREAD);	// always should be set 
	options.c_cflag &= ~CSIZE;

	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;

	case 2:
		options.c_cflag |= CSTOPB;
		break;

	default:
	{
		ASSERT(FALSE);
		return -1;
	}
	}

	switch (databits)
	{
	case 7:
		options.c_cflag |= CS7;
		break;

	case 8:
		options.c_cflag |= CS8;
		break;

	default:
	{
		ASSERT(FALSE);
		return -1;
	}
	}

	options.c_cflag &= ~CRTSCTS;				// 不使用硬件流控制
												/*
												IXON--启用输出的 XON/XOFF 流控制
												IXOFF--启用输入的 XON/XOFF 流控制
												IXANY--允许任何字符来重新开始输出
												IGNCR--忽略输入中的回车
												*/
	options.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | ICRNL);

	//options.c_iflag &= IGNCR; 				  // ignore CR 

	switch (parity)
	{
	case 0:
		options.c_cflag &= ~PARENB; 			/* Clear parity enable */
		break;

	case 1:
		options.c_cflag |= PARENB;				/* Enable parity */
		options.c_cflag |= PARODD;				/* 设置为奇校验 */
		break;

	case 2:
		options.c_cflag |= PARENB;				/* Enable parity */
		options.c_cflag &= ~PARODD; 			/* 转换为偶校验 */
		break;

	case 3:
		options.c_cflag &= ~PARENB; 			/* Enable parity */
		options.c_cflag |= CSTOPB;
		break;

	case 4: 									/* as no parity */
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;

	default:
	{
		ASSERT(FALSE);
		return -1;
	}
	}

	/* OPOST--启用具体实现自行定义的输出处理 */
	if (index == 1)
		options.c_oflag &= ~OPOST;

	/*
	ICANON--启用标准模式 (canonical mode)。允许使用特殊字符 EOF, EOL,
	EOL2, ERASE, KILL, LNEXT, REPRINT, STATUS, 和 WERASE，以及按行的缓冲。
	ECHO--回显输入字符
	ECHOE--如果同时设置了 ICANON，字符 ERASE 擦除前一个输入字符，WERASE 擦除前一个词
	ISIG--当接受到字符 INTR, QUIT, SUSP, 或 DSUSP 时，产生相应的信号
	*/
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw data

														/* VMIN--非 canonical 模式读的最小字符数
														VTIME--非 canonical 模式读时的延时，以十分之一秒为单位
														*/
	options.c_cc[VMIN] = 0;		// update the options and do it now 
	options.c_cc[VTIME] = 50;		// set timeout 5 seconds
	tcflush(fd, TCIFLUSH); /* TCIFLUSH Update the options and do it NOW */

						   /* TCSANOW--改变立即发生 */
	if (0 != tcsetattr(fd, TCSANOW, &options))
	{
		LogW(TAG,"fail tcsetattr,error=%d(%s)", errno, strerror(errno));
		//ASSERT(FALSE);
		return -1;
	}
#endif
	return 0;
}

void SerialPort_Linux::OnDestroy()
{
	__super::OnDestroy();

	if (mHandle >= 0)
	{
		_close(mHandle);
		mHandle = -1;
	}
}

void SerialPort_Linux::OnCreate()
{
	__super::OnCreate();

	string  filePath;

	if (mDeviceName.empty())
	{
#ifdef _CONFIG_MTK6572
		filePath = "/dev/ttyMT1";
#elif defined _CONFIG_INGENIC
		filePath = "/dev/ttyS0";
#endif
	}
	else
	{
		filePath = mDeviceName;
	}

	LogV(TAG, "try open[%s]", filePath.c_str());
	mHandle = _open(filePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

	int error = mHandle > 0 ? 0 : -1;
	SignalSerialOpenAck(this, error);

	LogV(TAG,"[%s],fd=%d", filePath.c_str(),mHandle);
	if (mHandle < 0)
	{
		LogW(TAG,"fail open %s", filePath.c_str());
		return;
	}

	{
		SetComSpeed(mHandle, mBaudRate);
	}

	auto s = mHandle;

	auto handle = (int)(LONGLONG)GetCurrentLooper()->GetLooperHandle();
	int ret = -1;
#ifdef __APPLE__
	ASSERT(FALSE);
#else
	struct epoll_event evt = { 0 };
	evt.events = EPOLLIN
		//| EPOLLOUT	//如果加上会一直触发此事件
		| EPOLLRDHUP
		| EPOLLERR
		;

	evt.data.ptr = (EpollProxy*)this;
	ret = epoll_ctl((int)handle, EPOLL_CTL_ADD, s, &evt);
#endif

	LogV(TAG,"serial ret=%d", ret);
	ASSERT(ret == 0);
}

}
}
