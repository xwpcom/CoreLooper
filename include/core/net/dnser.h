#pragma once
namespace Bear {
namespace Core {
namespace Net {

//XiongWanPing 2019.04.21
/*
提供异步域名解析,取代过时的DnsLooper
.支持缓存，可设定缓存时长
.支持快速退出app(可能引起误报memory leak)
 os提供的dns解析raw api都是阻塞接口,极端情况下可能阻塞几分钟或更长时间
 完全异步的dns解析是个枯燥无味的麻烦事情
 为保持简洁并且不依赖第三方库,在这里妥协一下,采用在raw thread阻塞调用os raw api来做
 好处是便于app能随时及时退出

 windows提供了WSAAsyncGetHostByName和WSACancelAsyncRequest
 Dnser设计与之类似
*/

class CORE_EXPORT Dnser :public Handler
{
	SUPER(Handler)

public:
	Dnser();
	~Dnser();

	int LOOPER_SAFE AsyncGetHostByName(const string& dns, std::weak_ptr<Handler> handler, std::function<void()>);
	void LOOPER_SAFE CancelAsyncRequest(std::weak_ptr<Handler> handler);
protected:
	void OnTimer(long id);

	struct tagItem
	{
		weak_ptr<Handler>		mHandler;
		std::function<void()>	mCB;
	};

	list< tagItem> mItems;

	long mTimerTest = 0;
};

}
}
}