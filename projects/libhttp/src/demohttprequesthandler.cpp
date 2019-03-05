#include "stdafx.h"
#include "demohttprequesthandler.h"
#include "net/channel.h"
#include "looper/loop.h"
#include "httptool.h"
using namespace Bear::Core::Net;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

DemoHttpRequestHandler::DemoHttpRequestHandler()
{
	SetObjectName("DemoHttpRequestHandler");
	DW("%s,this=0x%08x", __func__, this);

	mDumpFile.Open(ShellTool::GetAppPath() + "/http.recv.bin");
}

DemoHttpRequestHandler::~DemoHttpRequestHandler()
{
	DW("%s,this=0x%08x", __func__, this);
}

LRESULT DemoHttpRequestHandler::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		if (mChannel)
		{
			mChannel->Destroy();
			mChannel = nullptr;
		}
		break;
	}
	}

	auto ret = __super::OnMessage(msg, wp, lp);
	return ret;
}

void DemoHttpRequestHandler::OnClose(Channel*)
{
	DW("%s", __func__);

	if (mChannel)
	{
		mChannel->Destroy();
		mChannel = nullptr;
	}

	Destroy();
}

void DemoHttpRequestHandler::OnSend(Channel*)
{
	int x = 0;
}


void DemoHttpRequestHandler::OnConnect(Channel *endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo)
{
	if (error)
	{
		DW("%s,connect fail", __func__);

		Destroy();
		if (mChannel)
		{
			mChannel->Destroy();
			mChannel = nullptr;
		}
	}
	else
	{
		DV("%s,connect ok", __func__);

		mInbox.PrepareBuf(4 * 1024);
		mOutbox.PrepareBuf(4 * 1024);

		srand(1);

		SetTimer(mTimerCheckSend,1);
		SendRequest();
	}
}

void DemoHttpRequestHandler::OnReceive(Channel*)
{
	/*
	{
		static int idx = -1;
		++idx;
		DV("%s,idx=%04d", __func__, idx);
	}
	//*/

	while (mChannel)
	{
		mInbox.MoveToHead();
		int bytes = mInbox.GetTailFreeSize();
		if (bytes <= 1)
		{
			break;
		}

		int ret = mChannel->Receive(mInbox.GetNewDataPointer(), bytes - 1);
		if (ret <= 0)
		{
			break;
		}

		mInbox.WriteDirect(ret);
		mInbox.MakeSureEndWithNull();

		ParseInbox();
	}
}

//解析出inbox中所有的http ack,可能含多个ack
void DemoHttpRequestHandler::ParseInbox()
{
	ASSERT(mInbox.IsEndWithNull());

	while (!mInbox.IsEmpty())
	{
		switch (mHttpAckInfo.mHttpAckStatus)
		{
		case eHttpAckStatus_WaitHeader:
		{
			const char *ps = (const char*)mInbox.GetDataPointer();
			const char *key = "\r\n\r\n";
			const char *pEnd = strstr(ps, key);
			if (!pEnd)
			{
				return;
			}

			string  header(ps, pEnd - ps);

			{
				static int idx = -1;
				DV("S=>C [%04d] %s", ++idx, header.c_str());
			}

			mHttpAckInfo.mContentLength = HttpTool::GetInt(header, "Content-Length");
			if (mHttpAckInfo.mContentLength > 0)
			{
				SwitchStatus(eHttpAckStatus_ReceivingBody);
			}
			else
			{
				SwitchStatus(eHttpAckStatus_Done);
			}

			int eat = (int)(pEnd + strlen(key) - ps);
			mInbox.Eat(eat);
			break;
		}
		case eHttpAckStatus_ReceivingBody:
		{
			ASSERT(mHttpAckInfo.mContentRecvBytes <= mHttpAckInfo.mContentLength);

			int maxEatBytes = mHttpAckInfo.mContentLength - mHttpAckInfo.mContentRecvBytes;
			int bytes = mInbox.GetActualDataLength();
			int len = MIN(maxEatBytes, bytes);
			mHttpAckInfo.mContentRecvBytes += len;
			OnRecvHttpAckBody(mInbox.GetDataPointer(), len);
			mInbox.Eat(len);
			mInbox.MoveToHead();

			if (mHttpAckInfo.mContentRecvBytes == mHttpAckInfo.mContentLength)
			{
				mHttpAckInfo.Reset();
			}

			break;
		}
		case eHttpAckStatus_Done:
		{
			break;
		}
		default:
		{
			ASSERT(FALSE);
			break;
		}
		}
	}
}

void DemoHttpRequestHandler::OnRecvHttpAckBody(LPVOID data, int dataLen)
{
	mDumpFile.Write(data, dataLen);
}

void DemoHttpRequestHandler::SwitchStatus(DemoHttpRequestHandler::eHttpAckStatus status)
{
	mHttpAckInfo.mHttpAckStatus = status;
	switch (status)
	{
	case eHttpAckStatus_Done:
	{
		mHttpAckInfo.Reset();
		break;
	}
	}
}

void DemoHttpRequestHandler::SendRequest()
{
	if (!mChannel)
	{
		return;
	}

	ASSERT(mOutbox.IsEmpty());

	for (int i = 0; i < 10; i++)
	{
		string  req = StringTool::Format(
			"GET /UploadSensorData?id=00002XYZ&reqIndex=%d&temperature=30&humidity=57&windSpeed=0&windDirection=10&audioDB=35.0&ioInput=0&ioOutput=0&ioOutput=1&configVersion=2  HTTP/1.1\r\n"
			"\r\n"
			, mReqIndex++
		);

		{
			static int idx = -1;
			++idx;
			DV("C=>S [%04d] %s", idx, req.c_str());
			int len = (int)req.length();
			mOutbox.Write((LPVOID)req.c_str(), len);
		}
	}
}

void DemoHttpRequestHandler::OnTimer(long timerId)
{
	if(timerId == mTimerCheckSend)
	{
		if (!mChannel)
		{
			return;
		}

		if (mOutbox.IsEmpty())
		{
			SendRequest();
		}
		else
		{
			LPBYTE data = mOutbox.GetDataPointer();
			int bytes = MIN(1, mOutbox.GetActualDataLength());
			int ret = mChannel->Send(data, bytes);
			if (ret > 0)
			{
				mOutbox.Eat(ret);
			}

			//DV("%s", __func__);
			//int ms = rand() % 100;
			//ShellTool::Sleep(ms);
		}

		return;
	}

	__super::OnTimer(timerId);
}

}
}
}
}