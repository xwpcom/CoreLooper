#pragma once
#include "websocketsplitter.h"
#include "net/channel.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2019.06.06
class HTTP_EXPORT WebSocketHandler :public Handler, public WebSocketSplitter
{
	SUPER(Handler);
public:
	WebSocketHandler();
	~WebSocketHandler();
	sigslot::signal3<Handler*, LPBYTE, int>	SignalOnWebSocketRecv;
	sigslot::signal1 < Handler*>			SignalOnWebSocketClosed;
	sigslot::signal1<Handler*>				SignalOnWebSocketReadySend;
	virtual void Send(Handler*, ByteBuffer& box);
	void OnWSHandlerDestroy(Handler*);
	void Attach(shared_ptr<Channel> channel);

protected:
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	void CheckSend();
	void OnDestroy();

	void onWebSocketDecodePlayload(const WebSocketHeader& header, const uint8_t* ptr, uint64_t len, uint64_t recved);
	void onWebSocketEncodeData(const uint8_t* ptr, uint64_t len);

	shared_ptr<Channel>	mChannel;

	ByteBuffer mOutbox;
	ByteBuffer mWebSocketOutbox;//存放由onWebSocketEncodeData编码后的数据，直接发给对方websocket接收
};


}
}
}
}
