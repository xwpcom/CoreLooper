#pragma once
#include "looper/handler.h"
#include "libhttp/httpconfig.h"
#include "httphandler.h"
#include "Util/SSLBox.h"

namespace Bear {
namespace Core {
namespace Net {
class Channel;
namespace Http {
class HttpRequest;
using namespace toolkit;
//XiongWanPing 2019.11.22
class HTTP_EXPORT HttpsHandler :public HttpHandler
{
	friend class HttpsServer;
public:
	HttpsHandler();
	virtual ~HttpsHandler();
protected:
	virtual void OnConnect(Channel*, long, Bundle*);
	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);


	void ParseInbox();

	shared_ptr<BufferRaw> mInBuffer;
	shared_ptr<BufferRaw> mOutBuffer;
	SSL_Box _sslBox;
	ByteBuffer mInboxSSL;
	ByteBuffer mOutboxSSL;

	virtual ByteBuffer* GetBuffer_ParseInbox()
	{
		return &mInboxSSL;
	}
	virtual ByteBuffer* GetOutbox()
	{
		return &mOutboxSSL;
	}

	virtual void TransformOutboxData();
};

}
}
}
}
