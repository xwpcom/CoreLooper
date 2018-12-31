#include "stdafx.h"
#include "httphandler.h"
#include "base/log.h"
#include "httprequest.h"
#include "net/tcpclient.h"

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

		LPBYTE pData = mOutbox.GetDataPointer();
		int len = mOutbox.GetActualDataLength();
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
				mOutbox.Eat(ret);
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

			while (1)
			{
				if (!mHttpRequest)
				{
					/*
					if (strncmp((char*)buf, "OPTIONS", strlen("OPTIONS")) == 0
						|| strncmp((char*)buf, "DESCRIBE", strlen("DESCRIBE")) == 0
						)
					{
						//work as rtsp
						auto rtspServer = mWebConfig->mRtspServer.lock();
						if (rtspServer)
						{
							mChannel->SignalOnSend.disconnect(this);
							mChannel->SignalOnReceive.disconnect(this);
							mChannel->SignalOnClose.disconnect(this);

							ByteBuffer box;
							box.Write(buf, ret);
							DW("todo:rtsp");
							ASSERT(FALSE);
							//rtspServer->OnHttpRequestTransform(mChannel,box);
							mChannel->Destroy();
							mChannel = nullptr;
							Destroy();
							return;
						}
					}
					*/

					mHttpRequest = make_shared<CHttpRequest>();
					mHttpRequest->SetConfig(mWebConfig);
					mHttpRequest->SetOutbox(&mOutbox);
					mHttpRequest->SetPeerAddr(mChannel->GetPeerDesc());
					mHttpRequest->SetLocalAddr(mChannel->GetLocalDesc());
				}

				int bytes = mInbox.GetActualDataLength();
				ret = mHttpRequest->Input(mInbox);
				CheckSend();
				if (bytes == mInbox.GetActualDataLength())
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
}

}
}
}
}