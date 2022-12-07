#include "stdafx.h"
#include "httphandler.h"
#include "base/log.h"
#include "httprequest.h"
#include "net/tcpclient.h"
#include "websockethandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "HttpHandler";

HttpHandler::HttpHandler()
{
	SetObjectName("HttpHandler");

	mOutbox.PrepareBuf(1024 * 16);

	//LogV(TAG, "%s(%p)",__func__,this);
}

HttpHandler::~HttpHandler()
{
	//LogV(TAG, "%s(%p)", __func__, this);
}

void HttpHandler::OnConnect(Channel*, long error, Bundle*)
{
	mInbox.SetBufferSize(8 * 1024);
}

void HttpHandler::OnClose(Channel*)
{
	//LogV(TAG, "%s(%p)", __func__, this);

	Destroy();
}

void HttpHandler::OnSend(Channel*)
{
	CheckSend();
}

void HttpHandler::OnDestroy()
{
	__super::OnDestroy();

	//LogV(TAG, "%s(%p)", __func__, this);

	if (mChannel)
	{
		DetachChannel();

		mChannel->Destroy();
		mChannel = nullptr;
	}


}

void HttpHandler::CheckSend()
{
	if (mCheckSending)
	{
		return;
	}
	
	mCheckSending = true;
	class AutoReset
	{
	public:
		AutoReset(bool& v) :mValue(v)
		{

		}

		~AutoReset()
		{
			mValue = false;
		}

		bool& mValue;
	};

	AutoReset v(mCheckSending);

	auto outbox = GetOutbox();
	if (!outbox)
	{
		ASSERT(false);
		return;
	}

	while (1)
	{
		if (mHttpRequest)
		{
			mHttpRequest->Process();//可能提交新数据到mOutbox
			if (mHttpRequest->IsDone())
			{
				mHttpRequest = nullptr;
			}
		}

		if (outbox != &mOutbox)
		{
			TransformOutboxData();
		}

		LPBYTE pData = outbox->GetDataPointer();
		int len = outbox->GetActualDataLength();
		if (len <= 0)
		{
			break;
		}

		int ret = -1;
		auto obj = mChannel;
		if (obj)
		{
			ret = obj->Send(pData, len);
			if (ret > 0)
			{
				outbox->Eat(ret);
			}

			//大数据流量管控,比如下载较大文件时
			int maxCacheBytes = 8 * 1024;
			if (outbox->length() > maxCacheBytes || obj->GetOutboxCacheBytes() > maxCacheBytes)
			{
				return;
			}
		}

		if (ret <= 0)
		{
			break;
		}
	}
}

void HttpHandler::OnReceive(Channel*)
{
	//LogV(TAG, "%s(%p)", __func__, this);
	auto channel = mChannel;
	while (channel)
	{
		int freeSize = mInbox.GetFreeSize();
		if (freeSize < 2)//for easy parse,end with '\0'
		{
			LogW(TAG,"mInbox is full,dataLen=%d", mInbox.GetActualDataLength());
			return;
		}

		BYTE buf[1024 * 4];
		int len = MIN(sizeof(buf) - 1, freeSize - 1);
		ASSERT(len > 0);

		if (len > 0)
		{
			int ret = channel->Receive(buf, len);
			if (ret == 0)
			{
				//LogW(TAG,"socket is broken by client");

				channel->Close();
				return;
			}

			if (ret < 0)
			{
				return;
			}

			buf[ret] = 0;
			mInbox.Write(buf, ret);
			mInbox.MakeSureEndWithNull();

			ParseInbox();
		}
		else
		{
			break;
		}
	}
}

void HttpHandler::ParseInbox()
{
	auto inbox=GetBuffer_ParseInbox();
	if (!inbox)
	{
		ASSERT(FALSE);
		return;
	}

	auto channel = mChannel;
	while (1)
	{
		auto d = inbox->data();
		auto bytes = inbox->length();
		if (bytes == 0)
		{
			break;
		}

		if (!channel)
		{
			/*
			2022.01.13
			bug:
			.出现过HttpHandler已destroy(),但调用到HttpHandler::ParseInbox()
			如下语句访问到空的mChannel导致crash
			mHttpRequest->SetPeerAddr(mChannel->GetPeerDesc());
			原因待查
			*/
			return;
		}

		if (!mHttpRequest)
		{
			auto obj = make_shared<HttpRequest>();
			obj->SetConfig(mWebConfig);
			obj->SetOutbox(&mOutbox);
			obj->SetPeerAddr(channel->GetPeerDesc());
			obj->SetLocalAddr(channel->GetLocalDesc());
			mHttpRequest = obj;
		}

		auto httpResult = mHttpRequest;

		bytes = inbox->length();
		auto ret = httpResult->Input(*inbox);
		if (ret == -1)
		{
			Destroy();
			return;
		}

		CheckSend();

		if (httpResult->IsWebSocket())
		{
			auto url = httpResult->GetUrl();
			mHttpRequest = nullptr;

			if (mWebConfig->mWSFacotry)
			{
				auto obj = mWebConfig->mWSFacotry->CreateWebSocketHandler(url);
				if (obj)
				{
					AddChild(obj);

					DetachChannel();
					obj->Attach(channel);
					mChannel = nullptr;

					return;
				}
			}

			if (channel)
			{
				LogW(TAG,"no handler for websockt url:%s", url.c_str());

				channel->Destroy();
				Destroy();
				return;
			}

		}
		if (bytes == inbox->length())
		{
			break;
		}
	}
}

void HttpHandler::AttachChannel(Channel* obj)
{
	ASSERT(!mChannel);
	mChannel = dynamic_pointer_cast<TcpClient>(obj->shared_from_this());
	ASSERT(mChannel);

	obj->SignalOnSend.connect(this, &HttpHandler::OnSend);
	obj->SignalOnReceive.connect(this, &HttpHandler::OnReceive);
	obj->SignalOnClose.connect(this, &HttpHandler::OnClose);
}

void HttpHandler::DetachChannel()
{
	if (mChannel)
	{
		//LogV(TAG, "%s(%p)", __func__, this);

		mChannel->SignalOnSend.disconnect(this);
		mChannel->SignalOnReceive.disconnect(this);
		mChannel->SignalOnClose.disconnect(this);
	}
	else
	{
		LogW(TAG, "%s(%p),channel is null", __func__, this);
	}
}

}
}
}
}