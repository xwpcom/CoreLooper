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

HttpHandler::HttpHandler()
{
	SetObjectName("HttpHandler");
	//DV("%s", __func__);

	mOutbox.PrepareBuf(1024 * 16);
}

HttpHandler::~HttpHandler()
{
	//DV("%s", __func__);
}

void HttpHandler::OnConnect(Channel*, long error, Bundle*)
{
	mInbox.SetBufferSize(8 * 1024);
}

void HttpHandler::OnClose(Channel*)
{
	if (mChannel)
	{
		mChannel->Destroy();
		mChannel = nullptr;
	}

	Destroy();
}

void HttpHandler::OnSend(Channel*)
{
	//DV("%s", __func__);

	CheckSend();
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
		if (mChannel)
		{
			ret = mChannel->Send(pData, len);
			if (ret > 0)
			{
				outbox->Eat(ret);
			}

			//大数据流量管控,比如下载较大文件时
			int maxCacheBytes = 8 * 1024;
			if (outbox->length() > maxCacheBytes || mChannel->GetOutboxCacheBytes() > maxCacheBytes)
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
	//DV("%s", __func__);

	while (mChannel)
	{
		int freeSize = mInbox.GetFreeSize();
		if (freeSize < 2)//for easy parse,end with '\0'
		{
			DW("mInbox is full,dataLen=%d", mInbox.GetActualDataLength());
			return;
		}

		BYTE buf[1024 * 4];
		int len = MIN(sizeof(buf) - 1, freeSize - 1);
		ASSERT(len > 0);

		if (len > 0)
		{
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

#ifdef _MSC_VER
			if (0)
			{
				static DumpFile dump;
				if (!dump.IsOpen())
				{
					dump.Open(ShellTool::GetAppPath() + "/http/recv.bin");
				}
				dump.Write(buf, ret);
			}
#endif

			buf[ret] = 0;
			mInbox.Write(buf, ret);
			mInbox.MakeSureEndWithNull();

#ifdef _MSC_VER_DEBUG
			{
				//File::ReadFile("d:/recv.bin", mInbox);
			}
#endif
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

	while (1)
	{
		auto d = inbox->data();
		auto bytes = inbox->length();
		if (bytes == 0)
		{
			break;
		}
		if (!mHttpRequest)
		{
			mHttpRequest = make_shared<HttpRequest>();
			mHttpRequest->SetConfig(mWebConfig);
			mHttpRequest->SetOutbox(&mOutbox);
			mHttpRequest->SetPeerAddr(mChannel->GetPeerDesc());
			mHttpRequest->SetLocalAddr(mChannel->GetLocalDesc());
		}

		bytes = inbox->length();
		auto ret = mHttpRequest->Input(*inbox);
		CheckSend();
		if (mHttpRequest && mHttpRequest->IsWebSocket())
		{
			auto url = mHttpRequest->GetUrl();
			mHttpRequest = nullptr;

			if (mWebConfig->mWSFacotry)
			{
				mChannel->SignalOnSend.disconnect(this);
				mChannel->SignalOnReceive.disconnect(this);
				mChannel->SignalOnClose.disconnect(this);

				auto obj = mWebConfig->mWSFacotry->CreateWebSocketHandler(url);
				if (obj)
				{
					AddChild(obj);
					obj->Attach(mChannel);
					mChannel = nullptr;

					return;
				}
			}

			if (mChannel)
			{
				DW("no handler for websockt url:%s", url.c_str());

				mChannel->Destroy();
				mChannel = nullptr;
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

}
}
}
}