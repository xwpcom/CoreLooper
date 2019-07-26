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

			if (File::PathIsDirectory(tmpFilePath))
			{
				File::DeleteFolder(tmpFilePath.c_str());
			}

			File::CreateFolderForFile(tmpFilePath);
			mAckInfo.mFile = fopen(tmpFilePath.c_str(), "a+b");
			if (!mAckInfo.mFile)
			{
				Destroy();

				if (mDataEndPoint)
				{
					mDataEndPoint->Close();
					mDataEndPoint->Destroy();
				}

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
		int ret = mDataEndPoint->Send((LPVOID)req.c_str(), (int)len);
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
	switch (mAckInfo.mHttpAckStatus)
	{
	case eHttpAckStatus_WaitHeader:
	{
		mInbox.MakeSureEndWithNull();

		const char *ps = (const char*)mInbox.GetDataPointer();
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
					SwitchStatus(eHttpAckStatus_ReceivingBody);
				}
				else
				{
					SwitchStatus(eHttpAckStatus_Done);
				}
			}

			auto eat = pEnd + strlen(key) - ps;
			mInbox.Eat((int)eat);
			if (!mInbox.IsEmpty())
			{
				OnRecvHttpAckBody(mInbox.GetDataPointer(), mInbox.GetActualDataLength());
				mInbox.clear();
			}
		}
		break;
	}
	case eHttpAckStatus_ReceivingBody:
	{
		OnRecvHttpAckBody(mInbox.GetDataPointer(), mInbox.GetActualDataLength());
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
		mAckInfo.mBodyReceivedBytes += dataLen;
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

		long len = ftell(mAckInfo.mFile);
		//DV("len=%d,pending %d bytes", len, mAckInfo.mContentLength - len);
		if (len == mAckInfo.mContentLength)
		{
			fclose(mAckInfo.mFile);
			mAckInfo.mFile = nullptr;

			File::rename(mAckInfo.mSaveAsFilePath + ".tmp", mAckInfo.mSaveAsFilePath);

			//在mAckBody中返回下载保存的文件路径
			ByteBuffer& box = mAckInfo.mAckBody;
			box.clear();
			box.Write(mAckInfo.mSaveAsFilePath);
			box.MakeSureEndWithNull();

			SwitchStatus(eHttpAckStatus_Done);
		}
	}
	else
	{
		mAckInfo.mAckBody.Write(data, dataLen);

		bool done = false;
		if (mAckInfo.mChunked)
		{
			mAckInfo.mAckBody.MakeSureEndWithNull();
			auto p = (char*)mAckInfo.mAckBody.GetDataPointer();
			done = (strstr(p, "0\r\n\r\n") != nullptr);
		}
		else if (mAckInfo.mAckBody.GetActualDataLength() == mAckInfo.mContentLength)
		{
			done = true;
		}

		if (done)
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
		//mDumpFile.Close();
		if (mAckInfo.mChunked && !mAckInfo.mAckBody.IsEmpty())
		{
			//chunked格式:
			//16进制长度\r\n
			//数据\r\n
			//0\r\n
			//\r\n

			//下面去掉16进制长度\r\n和\r\n0\r\n\r\n,让mAckBody中只保留数据
			mAckInfo.mAckBody.MakeSureEndWithNull();
			auto& box = mAckInfo.mAckBody;
			auto ps = (const char*)box.GetDataPointer();
			auto bytes = strtol(ps, nullptr, 16);
			auto key = "\r\n";
			auto pStart = strstr(ps, key);
			if (pStart)
			{
				pStart += strlen(key);
				box.Eat((int)(pStart - ps));
				int tailBytes = box.GetActualDataLength() - bytes;
				if (tailBytes > 0)
				{
					box.ReverseEat(tailBytes);
				}
			}
		}

		{
			auto tickNow = ShellTool::GetTickCount64();
			if (tickNow > mAckInfo.mStartTick)
			{
				auto ms = tickNow - mAckInfo.mStartTick;
				auto speed = (double)(mAckInfo.mBodyReceivedBytes *1000.0/ ms/1024);
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
