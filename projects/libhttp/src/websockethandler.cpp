#include "stdafx.h"
#include "websockethandler.h"
#include "net/channel.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

WebSocketHandler::WebSocketHandler()
{
	SetObjectName("WebSocketHandler");
	DV("%s,this=%p", __func__, this);
}

WebSocketHandler::~WebSocketHandler()
{
	DV("%s,this=%p", __func__, this);
}

void WebSocketHandler::Attach(shared_ptr<Channel> channel)
{
	mChannel = channel;

	if (channel)
	{
		channel->SignalOnSend.connect(this, &WebSocketHandler::OnSend);
		channel->SignalOnReceive.connect(this, &WebSocketHandler::OnReceive);
		channel->SignalOnClose.connect(this, &WebSocketHandler::OnClose);
	}

	mOutbox.SetBufferSize(4 * 1024, 1024 * 1024);
	mWebSocketOutbox.SetBufferSize(4 * 1024, 2 * 1024 * 1024);
}

void WebSocketHandler::OnClose(Channel*)
{
	if (mChannel)
	{
		mChannel->Destroy();
		PostDispose(mChannel);
	}

	SignalOnWebSocketClosed(this);

	Destroy();
}

void WebSocketHandler::Send(Handler*, ByteBuffer& box)
{
	while (1)
	{
		mOutbox.MoveToHead();

		auto bytes = box.length();
		auto maxEatBytes = mOutbox.GetTailFreeSize();
		auto eatBytes = MIN(bytes, maxEatBytes);
		if (eatBytes > 0)
		{
			mOutbox.Write(box.data(), eatBytes);
			box.Eat(eatBytes);
		}

		CheckSend();

		if (box.empty() || !mOutbox.empty())
		{
			break;
		}
	}
}

void WebSocketHandler::OnSend(Channel*)
{
	SignalOnWebSocketReadySend(this);
	CheckSend();
}

//向websocket发数据之前要打包
//从websocket收到数据之后要解包
void WebSocketHandler::onWebSocketEncodeData(const uint8_t* ptr, uint64_t len)
{
	auto ret = mWebSocketOutbox.Write((LPBYTE)ptr, (int)len);
	if (ret != len)
	{
		DW("fail %s", __func__);
		Destroy();
	}
}

void WebSocketHandler::CheckSend()
{
	{
		LPBYTE pData = mOutbox.GetDataPointer();
		int len = mOutbox.GetActualDataLength();
		if (len > 0)
		{
			WebSocketHeader header;
			header._fin = true;
			header._reserved = 0;
			header._opcode = WebSocketHeader::TEXT;//这里要设置为TEXT,如果设置为BINARY,则微信小程序中websocket收到数据时，要自行转为string
			header._mask_flag = false;
			WebSocketSplitter::encode(header, (uint8_t*)pData, len);//编码数据保存在mWebSocketOutbox
			mOutbox.clear();
		}
	}

	auto& box = mWebSocketOutbox;
	while (box.length() > 0)
	{
		int ret = -1;
		if (mChannel)
		{
			ret = mChannel->Send(box.data(), box.length());
			if (ret > 0)
			{
				box.Eat(ret);
			}
		}

		if (ret <= 0)
		{
			break;
		}
	}
}

void WebSocketHandler::OnReceive(Channel*)
{
	while (mChannel)
	{
		BYTE buf[1024 * 8];
		int len = sizeof(buf) - 1;
		int ret = mChannel->Receive(buf, len);
		if (ret == 0)
		{
			//DW("socket is broken by client");
			mChannel->Close();
			return;
		}

		if (ret < 0)
		{
			return;
		}

		buf[ret] = 0;
		decode(buf, ret);
	}
}

void WebSocketHandler::onWebSocketDecodePlayload(const WebSocketHeader& header, const uint8_t* ptr, uint64_t len, uint64_t recved)
{
	int x = 0;
	auto type = header._opcode;
	if ((type & TEXT) || (type & BINARY))
	{
		//DV("this=%p,%s", this, (const char*)ptr);
		SignalOnWebSocketRecv(this, (LPBYTE)ptr, (int)len);
	}

	if (type & CLOSE)
	{
		//DV("this=%p,close",this);
		SignalOnWebSocketClosed(this);
	}
}

}
}
}
}
