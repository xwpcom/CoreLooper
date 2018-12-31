#pragma once
#include "net/tcpserver.h"
#include "rtsp_inc.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {

enum
{
	BM_BROADCAST_EVENT = 100,
};

//XiongWanPing 2016.03.30
class RTSP_CLASS RtspServer :public TcpServer
{
	SUPER(TcpServer)

public:
	RtspServer();
	virtual ~RtspServer();

	sigslot::signal2<RtspServer*, bool> SignalClientChanged;
	sigslot::signal2<RtspServer*, bool> SignalPlayStatusChanged;

	void OnPlayStatusChanged();
	virtual void SetConfig(std::shared_ptr<tagRtspServerConfig> config)
	{
		mRtspConfig = config;
	}

	virtual void OnHttpRequestTransform(std::shared_ptr<Channel> dataEndPoint, ByteBuffer& box);
	virtual void OnP2PAccept(std::shared_ptr<Channel> dataEndPoint);
protected:
	void OnCreate();
	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	void HandleConnect(std::shared_ptr<Channel> endPoint, long error, ByteBuffer* pBox, Bundle* extraInfo);
	void OnConnect(Channel* endPoint, long, ByteBuffer*, Bundle*);
	virtual std::shared_ptr<Channel> CreateChannel();

	void OnChildDetach();
	std::shared_ptr<tagRtspServerConfig> mRtspConfig;
	int								mConnectTimes;
	bool	mHasClient;//是否有客户端连接
	bool	mPlaying = false;
};

}
}
}
}
