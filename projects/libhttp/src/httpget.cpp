#include "stdafx.h"
#include "libhttp/httpget.h"
#include "net/dnslooper.h"
#include "net/tcpclient.h"
#include "string/utf8tool.h"

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

int HttpGet::Execute(const string& url, std::function<void(const string& url, int error, ByteBuffer& box)> fn)
{
	string saveAsFilePath;
	return Execute(url, saveAsFilePath, fn);
}

int HttpGet::Execute(const string& url, const string& saveAsFilePath, std::function<void(const string& url, int error, ByteBuffer& box)> fn)
{
	//LogV(mTag, "%s",__func__);

	ASSERT(IsMyselfThread());
	mCB = fn;

	if (!saveAsFilePath.empty())
	{
		if (File::FileExists(saveAsFilePath))
		{
			File::DeleteFile(saveAsFilePath.c_str());
		}

		if (File::FileExists(saveAsFilePath))
		{
			Destroy();
			return -1;
		}
	}

	mSignaled = false;
	mUrl = url;
	mAckInfo.Reset();
	mAckInfo.mSaveAsFilePath = saveAsFilePath;

	string host;
	int port = 0;
	string pageUrl;
	bool useHttps = false;
	int ret = HttpTool::ParseUrl(url, host, port, pageUrl,&useHttps);
	if (useHttps)
	{
		EnableTls();
	}

	if (mVerbose)
	{
		LogV(mTag, "useHttps=%d",useHttps);
	}

	mReqInfo.mPageUrl = pageUrl;
	mReqInfo.mHost = host;
	mReqInfo.mPort = port;

	if (mVerbose)
	{
		LogV(mTag, "%s(%s:%d)", __func__, host.c_str(),port);
	}

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

	if (mVerbose)
	{
		LogV(mTag, "%s(error=%d)",__func__,error);
	}

	auto channel = mChannel;
	if (error)
	{
		mAckInfo.mAckBody.MakeSureEndWithNull();

		if (!mSignaled)
		{
			mSignaled = true;
			if (mCB)
			{
				mCB(mUrl, -1, mAckInfo.mAckBody);
			}
			SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
		}

		Destroy();
	}
	else
	{
		mAckInfo.mStartTick = ShellTool::GetTickCount64();

		long startPos = 0;
		if (!mAckInfo.mSaveAsFilePath.empty())
		{
			//下载文件,需要支持断点续传
			string tmpFilePath = mAckInfo.mSaveAsFilePath + ".tmp";
			ASSERT(!mAckInfo.mFile);

			File::CreateFolderForFile(tmpFilePath);
			#ifdef _MSC_VER
			auto uFilePath = Utf8Tool::UTF8_to_UNICODE(tmpFilePath);
			mAckInfo.mFile = _wfopen(uFilePath, _T("wb"));
			#else
			mAckInfo.mFile = fopen(tmpFilePath.c_str(), "wb");
			#endif
			if (!mAckInfo.mFile)
			{
				Destroy();


				if (!mSignaled)
				{
					mSignaled = true;
					if (mCB)
					{
						mCB(mUrl, -1, mAckInfo.mAckBody);
					}
					SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
				}
				return;
			}

			startPos = ftell(mAckInfo.mFile);
		}

		string req=StringTool::Format(
			"%s %s HTTP/1.1\r\n"
			, mHttpAction.c_str()
			,mReqInfo.mPageUrl.c_str()
		);

		auto& items = mHeaders;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			auto name = iter->first;
			auto value = iter->second;

			req += StringTool::Format(
				"%s: %s\r\n"
				, name.c_str()
				, value.c_str()
			);
		}

		{
			auto it = mHeaders.find("Host");
			if (it == mHeaders.end())
			{
				req += StringTool::Format(
					"Host: %s\r\n"
					, mReqInfo.mHost.c_str()
				);
			}
		}
		if (mBodyRawData.length() > 0)
		{
			req += StringTool::Format("Content-Length: %d\r\n", (int)mBodyRawData.length());
		}

		req += "\r\n";

		if (mBodyRawData.length() > 0)
		{
			req += string((char*)mBodyRawData.data(), mBodyRawData.length());
		}

		if (mVerbose)
		{
			LogV(mTag, "C=>S %s", req.c_str());
		}

		//在android下面，有时网络不正常，但connect返回连接成功，此时send时会以EPIPE失败
		auto len = req.length();
		int ret = -1;
		
		if (channel)
		{
			ret = channel->Send((LPVOID)req.c_str(), (int)len);
		}

		if (ret == len)
		{
			mInbox.PrepareBuf(4 * 1024);
		}
		else
		{
			LogW(mTag, "fail send,len=%d,ret=%d",len,ret);

			mSignaled = true;
			if (mCB)
			{
				mCB(mUrl, -1, mAckInfo.mAckBody);
			}
			SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
			Destroy();
		}
	}
}

/*
2023.06.12
今天测试aleka路由器时，发现如下报文也符合http规范
特征是http ack中以\n\n结束header,然后跟一个text
00000000  47 45 54 20 2f 67 6f 66  6f 72 6d 2f 67 6f 66 6f   GET /gof orm/gofo
00000010  72 6d 5f 73 65 74 5f 63  6d 64 5f 70 72 6f 63 65   rm_set_c md_proce
00000020  73 73 3f 67 6f 66 6f 72  6d 49 64 3d 41 4c 4b 5f   ss?gofor mId=ALK_
00000030  4c 4f 47 49 4e 20 48 54  54 50 2f 31 2e 31 0d 0a   LOGIN HT TP/1.1..
00000040  48 6f 73 74 3a 20 31 39  32 2e 31 36 38 2e 31 2e   Host: 19 2.168.1.
00000050  31 0d 0a 0d 0a                                     1....
	00000000  48 54 54 50 2f 31 2e 31  20 32 30 30 20 4f 4b 0a   HTTP/1.1  200 OK.
	00000010  53 65 72 76 65 72 3a 20  44 65 6d 6f 2d 57 65 62   Server:  Demo-Web
	00000020  73 0d 0a 58 2d 46 72 61  6d 65 2d 4f 70 74 69 6f   s..X-Fra me-Optio
	00000030  6e 73 3a 20 53 41 4d 45  4f 52 49 47 49 4e 0a 50   ns: SAME ORIGIN.P
	00000040  72 61 67 6d 61 3a 20 6e  6f 2d 63 61 63 68 65 0a   ragma: n o-cache.
	00000050  43 61 63 68 65 2d 63 6f  6e 74 72 6f 6c 3a 20 6e   Cache-co ntrol: n
	00000060  6f 2d 63 61 63 68 65 0a  43 6f 6e 74 65 6e 74 2d   o-cache. Content-
	00000070  54 79 70 65 3a 20 74 65  78 74 2f 68 74 6d 6c 0a   Type: te xt/html.
	00000080  0a 7b 22 72 65 73 75 6c  74 22 3a 22 73 75 63 63   .{"resul t":"succ
	00000090  65 73 73 22 7d                                     ess"}
*/
void HttpGet::ParseInbox()
{
	KeepAlive();

	if(mVerbose && !mInbox.empty())
	{
		mInbox.MakeSureEndWithNull();

		LogV(mTag, "%s(%s)",__func__,(char*)mInbox.data());
	}
	
	switch (mAckInfo.mHttpAckStatus)
	{
	case eHttpAckStatus_WaitHeader:
	{
		mInbox.MakeSureEndWithNull();

		const char *ps = (const char*)mInbox.data();
		const char *key = "\r\n\r\n";
		const char *pEnd = strstr(ps, key);
		if (!pEnd)
		{
			key = "\n\n";
			pEnd = strstr(ps, key);
		}
		if (pEnd)
		{
			{
				string header(ps, pEnd - ps+strlen(key));
				HttpAcker obj;
				obj.Parse(header, true);
				mAckHeaders = obj.fields();
			}

			string header(ps, pEnd - ps);

			if (mVerbose)
			{
				LogV(mTag, "recv http header(%s)",header.c_str());
			}

			{
				HttpAckParser parser(header);
				const HttpAckParser::tagHttpAckInfo& info = parser.GetAckInfo();
				mAckInfo.mHttpAckCode = info.mAckCode;
			}

			mAckInfo.mContentLength = atoi(mAckHeaders["Content-Length"].c_str());// HttpTool::GetInt(header, "Content-Length");
			if (mAckInfo.mContentLength == 0)
			{
				mAckInfo.mContentLength = atoi(mAckHeaders["Content-length"].c_str());//todo:要支持大小写不敏感
			}
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

					auto data = mInbox.data();
					//2022.07.02 hot fix#begin,临时紧急使用
					{
						auto recvFinish = (strstr((char*)data, "\r\n0\r\n\r\n") != nullptr);

						if (recvFinish)
						{
							mAckInfo.mAckBody.Write(mInbox.data(), mInbox.length());
							mAckInfo.mAckBody.MakeSureEndWithNull();

							//extract data from chunked format
							SwitchStatus(eHttpAckStatus_Done);
							return;
						}
					}
					//2022.07.02 hot fix#end

					SwitchStatus(eHttpAckStatus_ReceivingChunkedLength);
					return;
				}
				else
				{
					bool done = true;
					if (mAckInfo.mAckBody.empty())
					{
						/*
						2023.06.13
						aleka 4G路由器http ack虽然也符合http规范，但比较另类
						每行可能以\n或\r\n结尾
						返回http body时可能不带Content-Length字段导致要做如下特殊处理
						根本原因是我们自己对Http协议没能完全支持和兼容,后续重构时改进
						*/

						auto text=pEnd + strlen(key);
						mAckInfo.mAckBody.Write((char*)text);

						auto aleka = strstr(ps, "Demo-Webs\r\n");//aleka Server
						if (aleka && !strstr(pEnd,"}"))
						{
							done = false;
							LogV(mTag, "need more data");
							mInbox.clear();
							SwitchStatus(eHttpAckStatus_ReceivingBody);
						}
					}

					if (done)
					{
						SwitchStatus(eHttpAckStatus_Done);
					}
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
		
		if (mVerbose)
		{
			LogV(mTag, "chunked.bytes=%d", bodyBytes);
		}
		
		//2024.04.20 hot fix#begin,临时紧急使用
		if (mAckInfo.mChunked)
		{
			auto data = mInbox.data();
			{
				auto recvFinish = (strstr((char*)data, "\r\n0\r\n\r\n") != nullptr);

				if (recvFinish)
				{
					mAckInfo.mAckBody.Write(mInbox.data(), mInbox.length());
					mAckInfo.mAckBody.MakeSureEndWithNull();

					//extract data from chunked format
					SwitchStatus(eHttpAckStatus_Done);
					return;
				}
			}
		}
		//2024.04.20 hot fix#end,临时紧急使用

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
			
			if (mVerbose)
			{
				LogV(mTag, "chunked=%d / %d,pending %d"
					, mAckInfo.mChunkedReceivedBytes
					, mAckInfo.mChunkedTotalBytes
					, pendingBytes
				);
			}

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
			
			if (mVerbose)
			{
				//LogV(mTag, "len=%d,pending %d bytes", len, mAckInfo.mContentLength - len);
			}

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

			SignalDownloadFileDone(this,mAckInfo.mSaveAsFilePath);
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

		if (!mSignaled)
		{
			mSignaled = true;

			if (mCB)
			{
				mCB(mUrl, 0, mAckInfo.mAckBody);
			}
			SignalHttpGetAck(this, mUrl, 0, mAckInfo.mAckBody);
		}

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
	if (mVerbose)
	{
		LogV(mTag, "%s(%p)", __func__, this);
	}

	__super::OnDestroy();

	if (!mSignaled)
	{
		mSignaled = true;
		if (mCB)
		{
			mCB(mUrl, -1, mAckInfo.mAckBody);
		}

		SignalHttpGetAck(this, mUrl, -1, mAckInfo.mAckBody);
	}
}

void HttpGet::AddHeader(const string& name, const string& value)
{
	mHeaders[name] = value;
}

void HttpGet::SetBody(const string& body)
{
	mBodyRawData.clear();
	mBodyRawData.Write(body);
}

void HttpGet::SetBodyRawData(const ByteBuffer& box)
{
	mBodyRawData.clear();
	mBodyRawData.Append(box);
}

}
}
}
}
