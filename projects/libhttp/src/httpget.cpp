#include "stdafx.h"
#include "libhttp/httpget.h"
#include "net/dnslooper.h"
#include "net/tcpclient.h"
using namespace Bear::Core::FileSystem;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {


HttpGet::HttpGet()
{
	mAckInfo.mAckBody.MakeSureEndWithNull();
}

HttpGet::~HttpGet()
{
}

int HttpGet::Execute(string url, string saveAsFilePath)
{
	ASSERT(IsMyselfThread());

	if (File::FileExists(saveAsFilePath))
	{
		File::DeleteFile(saveAsFilePath.c_str());
	}

	if (File::FileExists(saveAsFilePath))
	{
		Destroy();
		return -1;
	}

	mSignaled = false;
	mUrl = url;
	mAckInfo.Reset();
	mAckInfo.mSaveAsFilePath = saveAsFilePath;

	string host;
	int port = 0;
	string pageUrl;
	int ret = HttpTool::ParseUrl(url, host, port, pageUrl);

#ifdef _MSC_VER
	if (port == 80)
	{
		if (mUseTls)
		{
			port = 443;
		}
	}
#endif

	mReqInfo.mPageUrl = pageUrl;
	mReqInfo.mHost = host;
	mReqInfo.mPort = port;

	Bundle bundle;
	bundle.Set("address", host);
	bundle.Set("port", port);
	StartConnect(bundle);
	return 0;
}

void HttpGet::OnConnect(Channel *endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo)
{
	__super::OnConnect(endPoint, error, pBox, extraInfo);

	//已测试,windows iocp connect 20秒自动超时
	if (error)
	{
		mSignaled = true;
		mAckInfo.mAckBody.MakeSureEndWithNull();
		SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);

		Destroy();
	}
	else
	{
		//DV("%s,connect ok", __func__);

		mAckInfo.mStartTick = ShellTool::GetTickCount64();

		long startPos = 0;
		if (!mAckInfo.mSaveAsFilePath.empty())
		{
			//下载文件,需要支持断点续传
			string tmpFilePath = mAckInfo.mSaveAsFilePath + ".tmp";
			ASSERT(!mAckInfo.mFile);

			//if (File::PathIsDirectory(tmpFilePath))
			{
				//File::DeleteFolder(tmpFilePath.c_str());
			}

			File::CreateFolderForFile(tmpFilePath);
			mAckInfo.mFile = fopen(tmpFilePath.c_str(), "wb");
			if (!mAckInfo.mFile)
			{
				Destroy();


				mSignaled = true;
				SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
				return;
			}

			startPos = ftell(mAckInfo.mFile);
		}

		string req=StringTool::Format(
			"GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"\r\n"
			,
			mReqInfo.mPageUrl.c_str(),
			mReqInfo.mHost.c_str()
		);

		//DV("C=>S %s", req.c_str());

		//在android下面，有时网络不正常，但connect返回连接成功，此时send时会以EPIPE失败
		auto len = req.length();
		int ret = mChannel->Send((LPVOID)req.c_str(), (int)len);
		if (ret == len)
		{
			mInbox.PrepareBuf(4 * 1024);
		}
		else
		{
			mSignaled = true;
			SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
			Destroy();
		}
	}
}

void HttpGet::ParseInbox()
{
	KeepAlive();
	
	switch (mAckInfo.mHttpAckStatus)
	{
	case eHttpAckStatus_WaitHeader:
	{
		mInbox.MakeSureEndWithNull();

		const char *ps = (const char*)mInbox.data();
		const char *key = "\r\n\r\n";
		const char *pEnd = strstr(ps, key);
		if (pEnd)
		{
			string header(ps, pEnd - ps);

			{
				HttpAckParser parser(header);
				const HttpAckParser::tagHttpAckInfo& info = parser.GetAckInfo();
				mAckInfo.mHttpAckCode = info.mAckCode;
			}

			mAckInfo.mContentLength = HttpTool::GetInt(header, "Content-Length");
			if (mAckInfo.mContentLength > 0)
			{
				SwitchStatus(eHttpAckStatus_ReceivingBody);
			}
			else
			{
				{
					const char *key = "Transfer-Encoding:";
					auto encoding = strstr(ps, key);
					if (encoding)
					{
						auto p = strstr(encoding, " chunked\r\n");
						if (p)
						{
							mAckInfo.mChunked = true;
							//注意chunk长度是用十六进制字符串表示的
						}
					}
				}

				if (mAckInfo.mChunked)
				{
					auto eat = pEnd + strlen(key) - ps;
					mInbox.Eat((int)eat);
					SwitchStatus(eHttpAckStatus_ReceivingChunkedLength);
					return;
				}
				else
				{
					SwitchStatus(eHttpAckStatus_Done);
				}
			}

			int eat = (int)(pEnd + strlen(key) - ps);
			mInbox.Eat(eat);
			if (!mInbox.IsEmpty())
			{
				OnRecvHttpAckBody(mInbox.data(), mInbox.length());
				mInbox.clear();
			}
		}
		break;
	}
	case eHttpAckStatus_ReceivingChunkedLength:
	{
		mInbox.MakeSureEndWithNull();

		if (mAckInfo.mChunkedDoubleCRLF)
		{
			//格式为\r\nbytes\r\n
			//即必须包含两个\r\n
			auto data = (char*)mInbox.data();
			auto bytes = mInbox.length();
			if (bytes < 5)
			{
				return;
			}
			auto end = strstr(data+2, "\r\n");
			if (!end)
			{
				return;
			}

			ASSERT(data[0] == '\r');
			ASSERT(data[1] == '\n');
			mInbox.Eat(2);//eat first \r\n
			mAckInfo.mChunkedDoubleCRLF = false;
		}
		else
		{
			//格式为bytes\r\n
		}

		auto data = (char*)mInbox.data();
		auto bytes = mInbox.length();
		auto end = strstr(data, "\r\n");
		if (bytes < 3 || !end)
		{
			return;
		}

		//注意chunk长度是用十六进制字符串表示的
		auto bodyBytes = strtol(data, nullptr, 16);
		if (bodyBytes == 0)
		{
			mInbox.clear();
			SwitchStatus(eHttpAckStatus_Done);
			return;
		}

		mInbox.Eat((int)(end-data)+2);
		mAckInfo.mChunkedTotalBytes = bodyBytes;
		mAckInfo.mChunkedReceivedBytes = 0;
		DG("chunked.bytes=%d", bodyBytes);
		SwitchStatus(eHttpAckStatus_ReceivingChunkedBody);
		break;
	}
	case eHttpAckStatus_ReceivingChunkedBody:
	{
		auto data = mInbox.data();
		auto bytes = mInbox.length();
		auto eatBytes = (int)MIN(bytes, mAckInfo.mChunkedTotalBytes - mAckInfo.mChunkedReceivedBytes);
		if (eatBytes > 0)
		{
			OnRecvHttpAckBody(mInbox.data(), eatBytes);
			mInbox.Eat(eatBytes);

			mAckInfo.mChunkedReceivedBytes += eatBytes;

			auto pendingBytes = mAckInfo.mChunkedTotalBytes - mAckInfo.mChunkedReceivedBytes;
			DV("chunked=%d / %d,pending %d"
				, mAckInfo.mChunkedReceivedBytes
				, mAckInfo.mChunkedTotalBytes
				, pendingBytes
			);

			if (pendingBytes == 0)
			{
				int x = 0;
			}
		}

		if (mAckInfo.mChunkedReceivedBytes == mAckInfo.mChunkedTotalBytes)
		{
			mAckInfo.mChunkedTotalBytes = 0;
			mAckInfo.mChunkedReceivedBytes = 0;

			data = mInbox.data();

			mAckInfo.mChunkedDoubleCRLF=true;
			SwitchStatus(eHttpAckStatus_ReceivingChunkedLength);
			ParseInbox();
			return;
		}
		
		int x = 0;
		break;
	}
	case eHttpAckStatus_ReceivingBody:
	{
		OnRecvHttpAckBody(mInbox.data(), mInbox.length());
		mInbox.clear();
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

void HttpGet::OnRecvHttpAckBody(LPVOID data, int dataLen)
{
	if (dataLen > 0)
	{
		mAckInfo.mChunkedReceivedBytes += dataLen;
	}

	if (mAckInfo.mFile)
	{
		int ackCode = mAckInfo.mHttpAckCode;
		//206 is partial content
		if (ackCode != 200 && ackCode != 206)
		{
			Destroy();
			return;
		}

		auto ret = (long)fwrite(data, 1, dataLen, mAckInfo.mFile);
		if (ret != dataLen)
		{
			Destroy();
			return;
		}

		if (!mAckInfo.mChunked)
		{
			long len = ftell(mAckInfo.mFile);
			//DV("len=%d,pending %d bytes", len, mAckInfo.mContentLength - len);
			if (len == mAckInfo.mContentLength)
			{
				SwitchStatus(eHttpAckStatus_Done);
			}
		}
	}
	else
	{
		mAckInfo.mAckBody.Write(data, dataLen);
		mAckInfo.mAckBody.MakeSureEndWithNull();

		if (!mAckInfo.mChunked && mAckInfo.mAckBody.length() == mAckInfo.mContentLength)
		{
			mAckInfo.mAckBody.MakeSureEndWithNull();
			SwitchStatus(eHttpAckStatus_Done);
		}

	}
}

void HttpGet::SwitchStatus(HttpGet::eHttpAckStatus status)
{
	mAckInfo.mHttpAckStatus = status;
	switch (status)
	{
	case eHttpAckStatus_Done:
	{
		if (mAckInfo.mFile)
		{
			fclose(mAckInfo.mFile);
			mAckInfo.mFile = nullptr;

			File::rename(mAckInfo.mSaveAsFilePath + ".tmp", mAckInfo.mSaveAsFilePath);

			//在mAckBody中返回下载保存的文件路径
			ByteBuffer& box = mAckInfo.mAckBody;
			box.clear();
			box.Write(mAckInfo.mSaveAsFilePath);
			box.MakeSureEndWithNull();
		}

		{
			auto tickNow = ShellTool::GetTickCount64();
			if (tickNow > mAckInfo.mStartTick)
			{
				auto ms = tickNow - mAckInfo.mStartTick;
				auto speed = (double)(mAckInfo.mChunkedReceivedBytes *1000.0/ ms/1024);
				mAckInfo.mSpeed = speed;
			}
		}

		mSignaled = true;
		SignalHttpGetAck(this, mUrl, 0, mAckInfo.mAckBody);

		Destroy();
		break;
	}
	default:
	{
		break;
	}
	}
}

void HttpGet::OnDestroy()
{
	__super::OnDestroy();

	if (!mSignaled)
	{
		mSignaled = true;
		SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
	}
}

}
}
}
}
