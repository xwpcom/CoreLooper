#include "stdafx.h"
#include "httppost.h"
#include "net/tcpclient.h"
#include "mime.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

using namespace Core;

static const char* TAG = "HttpPost";

//static int gBigFrameIndex = 0;

HttpPost::HttpPost()
{
	//LogW(TAG,"%s,this=%p", __func__, this);
	SetObjectName("HttpPost");
	mBoundary = "--------------------------716657498184592405569245";
	//LogV(TAG, "%s,this=%p", __func__, this);
}

HttpPost::~HttpPost()
{
	//LogV(TAG,"%s,this=%p", __func__, this);

	if (!mRecvAckDone)
	{
		if (mPostAckHandler)
		{
			mPostAckHandler->OnPostFail(-1, "");
		}
	}
}

void HttpPost::SetServerPort(string server, int port)
{
	//LOGV(TAG,"%s(%s:%d)", __func__, server.c_str(), port);
	mServer = server;
	mPort = port;
}

void HttpPost::AddHeader(string name, string value)
{
	mHeaders[name] = value;
}

void HttpPost::AddField(string name, string value)
{
	tagItem item;
	item.name = name;
	item.value = value;
	mFields.push_back(item);
}

//filePath必须为存在的文件
int HttpPost::AddFile(string name, string filePath)
{
	if (!File::FileExists(filePath.c_str()))
	{
		LogW(TAG, "skip AddFile(%s,%s),file no found:", name.c_str(), filePath.c_str());
		//ASSERT(FALSE);
		return -1;
	}

	tagItem item;
	item.name = name;
	item.value = filePath;

	FILE* file = fopen(filePath.c_str(), "rb");
	auto obj = shared_ptr<FILE>(file, ::fclose);

	item.mBytes = File::GetFileLength(file);
	//LogV(TAG,"AddFile(%s),bytes=%d",filePath.c_str(),item.mBytes);
	mFiles.push_back(item);
	return 0;
}

int HttpPost::Start(string url)
{
	ASSERT(!mChannel && !mConnected);
	mUrl = url;

	Bundle info;
	info.Set("address", mServer);
	info.Set("port", mPort);

	//LOGI(TAG, "connect %s:%d",info.GetString("address").c_str(),info.GetInt("port"));

	StartConnect(info);
	return 0;
}

void HttpPost::OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo)
{
	__super::OnConnect(endPoint, error, box, extraInfo);

	if (error == 0)
	{
		PrepareData();
	}
}

#ifdef _DEBUG
//一次性生成,仅供测试使用
int HttpPost::PackData()
{
	string header;
	ByteBuffer body;
	const auto& boundary = mBoundary;
	{
		header += StringTool::Format(
			"POST %s HTTP/1.1\r\n"
			, mUrl.c_str()
		);

		auto& items = mHeaders;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			auto name = iter->first;
			auto value = iter->second;

			header += StringTool::Format(
				"%s: %s\r\n"
				, name.c_str()
				, value.c_str()
			);
		}

		//todo:find ignore case
		if (items.find("Content-Type") == items.end()
			&& items.find("Content-type") == items.end()//dahua ipcam
			)
		{
			header += StringTool::Format(
				"Content-Type: multipart/form-data; boundary=%s\r\n"
				, boundary.c_str()
			);
		}

		if (items.find("Host") == items.end())
		{
			bool addPort = true;
			bool useTls = false;
		#ifdef _MSC_VER
			useTls = mUseTls;
		#endif
			if ((useTls && mPort == 443) ||(!useTls && mPort == 80))
			{
				addPort = false;
			}

			if (addPort)
			{
				header += StringTool::Format(
					"Host: %s:%d\r\n"
					, mServer.c_str()
					, mPort
				);
			}
			else
			{
				header += StringTool::Format(
					"Host: %s\r\n"
					, mServer.c_str()
				);
			}
		}

		if (items.find("Connection") == items.end())
		{
			header += StringTool::Format(
				"Connection: keep-alive\r\n"
			);
		}
	}

	{
		auto& items = mFields;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			auto& name = iter->name;
			auto& value = iter->value;

			body.Write(StringTool::Format(
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"\r\n"
				"\r\n"
				"%s\r\n"
				, boundary.c_str()
				, name.c_str()
				, value.c_str()
			));
		}
	}

	{
		auto& items = mFiles;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			ByteBuffer box;
			File::ReadFile(iter->value.c_str(), box);

			body.Write(StringTool::Format(
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
				"Content-Type: %s\r\n"
				"\r\n"
				, boundary.c_str()
				, iter->name.c_str()
				, File::GetPathFileName(iter->value.c_str()).c_str()
				, Mime::GetFileContentType(iter->value).c_str()
			));
			body.Append(box);
			body.Write("\r\n");

			auto fileBytes = File::GetFileLength(iter->value.c_str());
			//body.Write(desc);
			//LogV(TAG,"desc.len=%d", desc.length());
			//LogV(TAG,"file.len=%d", fileBytes);
		}
	}
	int contentLength = body.GetDataLength();

	auto tail = StringTool::Format(
		"--%s--\r\n"
		, boundary.c_str()
	);
	//LogV(TAG,"tail.len=%d", tail.length());
	contentLength += tail.length();

	ByteBuffer req;

	req.Write(StringTool::Format(
		"%s"
		"Content-Length: %d\r\n"
		"\r\n"
		, header.c_str()
		, contentLength
	));
	req.Append(body);
	req.Write(tail);

#ifdef _MSC_VER_DEBUG
	File::Dump(req, "G:/test/hz/httppost.bear.txt");
#endif

	SwitchStage(eSendHeader);
	return 0;
}
#endif

int HttpPost::PrepareData()
{
	string header;

	ByteBuffer body;
	const auto& boundary = mBoundary;
	{
		header += StringTool::Format(
			"%s %s HTTP/1.1\r\n"
			, mHttpMethod.c_str()
			, mUrl.c_str()
		);

		auto& items = mHeaders;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			auto name = iter->first;
			auto value = iter->second;

			header += StringTool::Format(
				"%s: %s\r\n"
				, name.c_str()
				, value.c_str()
			);
		}

		if (items.find("Content-Type") == items.end()
			&& items.find("Content-type") == items.end()
			)
		{
			header += StringTool::Format(
				"Content-Type: multipart/form-data; boundary=%s\r\n"
				, boundary.c_str()
			);
		}

		if (items.find("Host") == items.end())
		{
			bool addPort = true;
			bool useTls = false;

		#ifdef _MSC_VER
			useTls = mUseTls;
		#endif
			if ((useTls && mPort == 443) || (!useTls && mPort == 80))
			{
				addPort = false;
			}

			if (addPort)
			{
				header += StringTool::Format(
					"Host: %s:%d\r\n"
					, mServer.c_str()
					, mPort
				);
			}
			else
			{
				header += StringTool::Format(
					"Host: %s\r\n"
					, mServer.c_str()
				);
			}
		}

		if (items.find("cache-control") == items.end())
		{
			header += "cache-control: no-cache\r\n";
		}

		if (items.find("User-Agent") == items.end())
		{
			//header += "User-Agent: CoreLooper\r\n";
		}

		if (items.find("Connection") == items.end())
		{
			header += StringTool::Format(
				"Connection: keep-alive\r\n"
			);
		}
	}

	bool json = false;
	{
		if (mHeaders.find("Content-Type") != mHeaders.end())
		{
			json = (mHeaders["Content-Type"].find("application/json") != -1);
		}

	}

	const bool postFields = IsPostFileFields();

	ASSERT(!(json && postFields));
	int contentLength = 0;
	if (!mBodyBigFilePath.empty())
	{
		FILE* file = fopen(mBodyBigFilePath.c_str(), "rb");
		if (file)
		{
			auto obj = shared_ptr<FILE>(file, ::fclose);
			mBigFile = obj;
			contentLength = (int)File::GetFileLength(file);
		}
	}
	else if (json || !mBodyRawData.empty())
	{
		contentLength = mBodyRawData.GetDataLength();
	}
	else
	{
		auto& items = mFields;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			auto& name = iter->name;
			auto& value = iter->value;

			body.Write(StringTool::Format(
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"\r\n"
				"\r\n"
				"%s\r\n"
				, boundary.c_str()
				, name.c_str()
				, value.c_str()
			));
		}

		contentLength = body.GetDataLength();
	}
	//header和fields一般较小，可直接放在mOutbox中,
	//考虑到要支持发送超大的文件，不能一次性读取文件到内存，所以files采用渐进发送
	//这里要计算出总的content-length

	if (postFields)
	{
		auto& items = mFiles;
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			//ByteBuffer box;
			//File::ReadFile(iter->value.c_str(), box);

			auto desc = StringTool::Format(
				"--%s\r\n"
				"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
				"Content-Type: %s\r\n"
				"\r\n"
				, boundary.c_str()
				, iter->name.c_str()
				, File::GetPathFileName(iter->value.c_str()).c_str()
				, Mime::GetFileContentType(iter->value).c_str()
			);
			contentLength += desc.length();
			auto fileBytes = File::GetFileLength(iter->value.c_str());//file data
			contentLength += fileBytes;
			contentLength += strlen("\r\n");
		}
	}

	if (postFields)
	{
		auto tail = StringTool::Format(
			"--%s--\r\n"
			, boundary.c_str()
		);
		//LogV(TAG,"tail.len=%d", tail.length());
		contentLength += tail.length();
		//LogV(TAG,"contentLength=%d", contentLength);
	}

	ByteBuffer req;
	req.Write(StringTool::Format(
		"%s"
		"Content-Length: %d\r\n"
		"\r\n"
		, header.c_str()
		, contentLength
	));
	req.Append(body);
	req.MakeSureEndWithNull();
	//req.Write(tail);

	mOutbox.Append(req);
	SwitchStage(eSendHeader);
	//CheckSend();
	return 0;
}

void HttpPost::SwitchStage(HttpPost::eSendStage stage)
{
	if (stage!= eSendBodyRaw && mInfo.mStage == stage)
	{
		return;
	}

	mInfo.mStage = stage;
	//LogV(TAG,"SwitchStage(%s)", GetStageDesc(stage));

	if (stage == eSendFinish)
	{
		KeepAlive();
	}
	else
	{
		PreStage(mInfo.mStage);
	}
}

//要与eHttpError同步
const char* HttpPost::GetStageDesc(HttpPost::eSendStage v)
{
	static const char* arr[] =
	{
#undef ITEM
#define ITEM(x)	(char*)x,#x
		ITEM(eSendNone),
		ITEM(eSendHeader),
		ITEM(eSendFileDesc),//file描述头
		ITEM(eSendFileBody),//file data
		ITEM(eSendFileCRLF),//file结尾的\r\n
		ITEM(eSendTail),
		ITEM(eSendBodyRaw),
		ITEM(eSendFinish),
#undef ITEM
	};

	for (UINT i = 0; i < COUNT_OF(arr); i += 2)
	{
		if ((char*)v == arr[i])
		{
			return arr[i + 1];
		}
	}

	ASSERT(FALSE);
	return "Unknown";
}

//当outbox中的数据发送完成，可提交要写的新数据时会调用本接口
void HttpPost::OnOutboxWritable()
{
	//LogV(TAG, "%s",__func__);

	ASSERT(mOutbox.GetActualDataLength() == 0);

	if (mInfo.mStage != eSendFinish)
	{
		PostStage(mInfo.mStage);
	}

	KeepAlive();
}

void HttpPost::PreStage(HttpPost::eSendStage stage)
{
	//LogV(TAG,"PreStage(%s)", GetStageDesc(stage));
	switch (stage)
	{
	case eSendHeader:
	{
		CheckSend();
		return;
	}

	case eSendFileDesc:
	{
		++mInfo.mFileIndex;
		auto& items = mFiles;
		if (mInfo.mFileIndex < (int)items.size())
		{
			auto fileInfo = items[mInfo.mFileIndex];
			{
				auto desc = StringTool::Format(
					"--%s\r\n"
					"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
					"Content-Type: %s\r\n"
					"\r\n"
					, mBoundary.c_str()
					, fileInfo.name.c_str()
					, File::GetPathFileName(fileInfo.value.c_str()).c_str()
					, Mime::GetFileContentType(fileInfo.value).c_str()
				);
				mOutbox.Write(desc);
				//contentLength += desc.length();
				auto fileBytes = File::GetFileLength(fileInfo.value.c_str());//file data
				//contentLength += fileBytes;
				//contentLength += strlen("\r\n");
				{
					FILE* file = fopen(fileInfo.value.c_str(), "rb");
					if (file)
					{
						mInfo.mFile = shared_ptr<FILE>(file, ::fclose);
						//LogV(TAG,"fopen(%s),bytes=%d", fileInfo.value.c_str(), fileBytes);
						if (fileInfo.mBytes != fileBytes)
						{
							//android平板大华camera结束录像后
							//cached fileInfo.mBytes=180224 not equal current bytes=200704
							//
							LogW(TAG,"cached fileInfo.mBytes=%d not equal current bytes=%d", fileInfo.mBytes, fileBytes);
							ASSERT(FALSE);
							Destroy();
							return;
						}

						//LogV(TAG,"fopen(%s),fileBytes=%d", fileInfo.value.c_str(), fileBytes);
					}
					else
					{
						LogW(TAG,"fail fopen %s", fileInfo.value.c_str());
						Destroy();
						return;
					}
				}

				CheckSend();
			}
		}
		else
		{
			SwitchStage(eSendTail);
		}
		break;
	}
	case eSendFileBody:
	{
		ASSERT(mInfo.mFile);

		auto bytes = mOutbox.GetTailFreeSize();
		if (bytes > 0)
		{
			auto p = mOutbox.GetNewDataPointer();
			auto ret = fread(p, 1, bytes, mInfo.mFile.get());
			if (ret > 0)
			{
				//LogV(TAG,"fread bytes =%d",ret);
				mOutbox.WriteDirect(ret);
			}
			else if (ret <= 0)
			{
				mInfo.mFile = nullptr;
				SwitchStage(eSendFileCRLF);
				return;
			}
		}

		CheckSend();
		break;
	}
	case eSendFileCRLF:
	{
		mOutbox.Write("\r\n");
		CheckSend();
		break;
	}
	case eSendTail:
	{
		auto tail = StringTool::Format(
			"--%s--\r\n"
			, mBoundary.c_str()
		);
		mOutbox.Write(tail);
		CheckSend();
		break;
	}
	case eSendBodyRaw:
	{
		if (mBigFile)
		{
			auto bytes = mOutbox.GetTailFreeSize();
			if (bytes > 0)
			{
				auto p = mOutbox.GetNewDataPointer();
				auto ret = fread(p, 1, bytes, mBigFile.get());
				if (ret > 0)
				{
					/*
					{
						++gBigFrameIndex;
						LogV(TAG, "[%04d]fread bytes =%d,pos=%d,remainingBytes=%d"
							, gBigFrameIndex,ret, ftell(mBigFile.get())
							,File::GetFileLength(mBigFile.get()) - ftell(mBigFile.get())
						);
					}
					*/

					mOutbox.WriteDirect(ret);
					CheckSend();
				}
				else
				{
					if (feof(mBigFile.get()))
					{
						LogV(TAG, "bigFile reach end");
						SwitchStage(eSendFinish);
					}
				}
			}
		}
		else if(mBodyBigFilePath.empty())
		{
			mOutbox.Append(mBodyRawData);
			mBodyRawData.clear();
		}
		break;
	}
	}

}

//完成当前发送任务时调用本接口，一般是转到一下stage
void HttpPost::PostStage(eSendStage stage)
{
	//LogV(TAG,"PostStage(%s)", GetStageDesc(stage));

	switch (stage)
	{
	case eSendHeader:
	{
		if (IsPostFileFields())
		{
			if (mFiles.size() > 0)
			{
				SwitchStage(eSendFileDesc);
			}
			else
			{
				SwitchStage(eSendTail);
			}
		}
		else
		{
			SwitchStage(eSendBodyRaw);
		}
		break;
	}
	case eSendFileDesc:
	{
		SwitchStage(eSendFileBody);
		break;
	}
	case eSendFileBody:
	{
		PreStage(eSendFileBody);//继续读取文件
		break;
	}
	case eSendFileCRLF:
	{
		if (mInfo.mFileIndex < (int)mFiles.size() - 1)
		{
			SwitchStage(eSendFileDesc);
		}
		else
		{
			SwitchStage(eSendTail);
		}
		break;
	}
	case eSendTail:
	case eSendBodyRaw:
	{
		if (mBigFile)
		{
			if (!feof(mBigFile.get()))
			{
				SwitchStage(eSendBodyRaw);
				return;
			}
		}

		SwitchStage(eSendFinish);
		break;
	}
	case eSendFinish:
	{
		if (mBigFile)
		{
			mBigFile = nullptr;
		}

		break;
	}
	}
}

void HttpPost::ParseInbox()
{
	if (mRecvAckDone)
	{
		return;
	}

	mInbox.MakeSureEndWithNull();
	const char* ack = (const char*)mInbox.GetDataPointer();
	const char* key = "\r\n\r\n";
	auto headerTail = strstr(ack, key);
	auto headerReady = (headerTail != nullptr);
	int receivedBodyBytes = 0;
	if (headerTail)
	{
		auto headerBytes = headerTail + strlen(key) - ack;
		receivedBodyBytes = mInbox.GetDataLength() - headerBytes;
	}
	bool chunked = false;
	bool recvFinish = false;//是否收到完整的回复
	if (headerReady)
	{
		int contentLength = 0;
		{
			const char* key = "Content-Length: ";
			auto ps = strstr(ack, key);
			if (ps)
			{
				contentLength = atoi(ps + strlen(key));
			}
		}

		if (contentLength == 0)
		{
			//todo:parse all ack header and compare ignore case
			chunked = (strstr(ack, "Transfer-Encoding: chunked\r\n") != nullptr)
				|| (strstr(ack, "transfer-encoding: chunked\r\n") != nullptr)
				;

			if (chunked)
			{
				recvFinish = (strstr(ack, "\r\n0\r\n\r\n") != nullptr);

				if (recvFinish)
				{
					//extract data from chunked format
				}
			}
			else
			{
				recvFinish = true;
			}
		}
		else
		{
			recvFinish = (receivedBodyBytes >= contentLength);
		}
	}

	//	LogV(TAG,"recvFinish=%d", recvFinish);
	if (recvFinish)
	{
		/*
		{
			//andriod studio logcat在遇到超长log时会截断，所以这是分段输出
			string msg = (const char*)mInbox.GetDataPointer();
			int idx = -1;
			auto len = msg.length();
			int offset = 0;
			const int lineSize = 512;
			while (len > 0)
			{
				++idx;
				auto bytes = MIN(len - 1, lineSize);
				if (bytes == 0)
				{
					break;
				}
				LogV(TAG,"bytes=%d#begin", bytes);
				auto line = msg.substr(offset, bytes);
				LogV(TAG,"bytes=%d#end", bytes);
				offset += bytes;
				len -= bytes;
				LogV(TAG,"http post ack.%02d=[%s]", idx, line.c_str());
			}
		}
		//*/

		mRecvAckDone = true;
		OnRecvHttpAckDone();
		Destroy();
	}
}

//收到完整的http ack时会调用本接口，数据在mInbox
void HttpPost::OnRecvHttpAckDone()
{
	//LOGV(TAG,"%s", __func__);

	if (mPostAckHandler)
	{
		string ack = (const char*)mInbox.GetDataPointer();
		mPostAckHandler->OnPostAck(ack);
	}
}

void HttpPost::SetBodyRawData(const ByteBuffer& box)
{
	mBodyRawData.clear();
	mBodyRawData.SetBufferSize(16 * 1024, box.length() + 1);
	mBodyRawData.Append(box);
	mBodyRawData.MakeSureEndWithNull();
}

void HttpPost::SetBody(const string& text)
{
	mBodyRawData.clear();
	mBodyRawData.Write(text);
	mBodyRawData.MakeSureEndWithNull();
}

//2022.03.02华为云上报大文件用到
void HttpPost::SetBodyBigFile(const string& filePath)
{
	mBodyBigFilePath = filePath;
}


}
}
}
}
