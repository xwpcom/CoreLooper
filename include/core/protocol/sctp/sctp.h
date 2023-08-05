#pragma once
extern "C"
{
#include "csctp.h"
}

namespace SCTP
{

using namespace Bear::Core;

//XiongWanPing 2018.12.25
//Sctp供c++ app使用,可用在sim868上面
//注意不要使用c++11,sim868编译器不支持c++11
//tagSCTP供单片机使用,单片机不支持c++,只能用C语言
class 
#ifndef _CONFIG_SERIAL_CAMERA	
	CORE_EXPORT
#endif	
	Sctp:public Handler
{
	SUPER(Handler)
public:
	Sctp();

	tagSCTP& sctp()
	{
		return mObject;
	}
	void setTag(const string& tag)
	{
		mTag = tag;
	}

	void EnableSeq()
	{
		mEnableSeq = true;
	}

	void DisableSeq()
	{
		mEnableSeq = false;
	}

	void disableDumpCrcError()
	{
		mDumpCrcError = false;
	}

	void Create();
	void clear();
	void Enable485VerboseMode();

	//receiver专用,parser
	int  InputString(char *text);
	int  InputData(void *text, unsigned short bytes);

	//sender专用,creater
	void PrepareCreateOutboxData();
	void PrepareOutbox()
	{
		PrepareCreateOutboxData();
	}

	//注意:为节省内存,AddField直接引用数据,没有自行复制，所以app要保证name和value持续有效,直到调用CreateOutboxData之后，name和value才能失效
	int AddField(const char *name, const char *value);
	
	const tagByteBuffer& CreateAckOutboxData(tagBundle * activeBundle);
	const tagByteBuffer& CreateOutboxData(int* ackSeq=nullptr);

	sigslot::signal3<Sctp*, const char*, tagBundle *> SignalOnRecvCommand;

protected:
	static void SCTP_OnRecvCommandCB_(tagSCTP *obj, const char *cmd, tagBundle *bundle);
	static void SCTP_OnErrorCB_(tagSCTP *obj, const char *desc);

	void OnRecvCommand(const char *cmd, tagBundle *bundle);
	void OnError(tagSCTP* obj,const char *desc);
	int  CheckCrc();
	
	tagSCTP mObject;

	//可能用在sim868上面,所以采用静态buffer,避免在运行时频繁的alloc/free内存
	//注:下面这些字段仅在Create()中初始化用到，其他地方都是通过mObject来使用的
	unsigned char	mInbox[3000];//2019.11.29从1500增大为3000,是为一次性返回solar.TaskManager中的task list
	unsigned char	mOutbox[3000];//2019.11.29从1500增大为3000
	tagKeyValue		mInboxItems[16];
	tagKeyValue		mOutboxItems[16];

	bool mEnableSeq;
	string mTag = "sctp";
	bool mDumpCrcError = true;
};

#ifdef _MSC_VER

class CBundle
{
public:
	tagBundle		mBundle;
	CBundle()
	{
		Bundle_Create(&mBundle, mItems, COUNT_OF(mItems));
	}
protected:
	tagKeyValue		mItems[16];
};

#endif

}
