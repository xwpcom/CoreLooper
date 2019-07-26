#pragma once
namespace Bear {
namespace Core
{
namespace Net {

//XiongWanPing 2016.08.16
//提供异步域名解析
//已过时，建议使用Dnser
class CORE_EXPORT DnsLooper :public Bear::Core::Looper
{
	SUPER(Looper)

public:
	DnsLooper();
	~DnsLooper();

	//Add和Cancel内部通过sendMessage实现，所以是线程安全的
	int LOOPER_SAFE AddRequest(std::string dns, std::weak_ptr<Handler> handler, UINT msg);
	void LOOPER_SAFE CancelRequest(std::weak_ptr<Handler> handler);

protected:
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	void OnTimer(long timerId);
	int StartDnsThread();
	static void* WINAPI _DnsThreadCB(void *p);
	void OnDnsParseDone(std::string dns, std::string ip);

protected:
	struct tagItemInfo
	{
		tagItemInfo()
		{
			mCancel = false;
		}

		std::string				mDns;
		std::weak_ptr<Handler>	mHandler;
		UINT					mMsg;
		bool					mCancel;
	};

	std::list<tagItemInfo> mItems;
	long mTimerCheck = 0;

	//DnsLooper采用单独的工作线程(不是looper)来调用阻塞性api做解析
	//此工作线程只存取static变量，和DnsLooper没有依赖关系
	//DnsLooper可随时退出,不用等工作线程结束
	//同一app中只允许有一个DnsLooper实例
	//*
	static bool		mThreadRunning;
	static char		mDns[128];//工作线程输入
	static char		mIP[16];//存放工作线程解析出的ip
	//*/
};
}
}
}