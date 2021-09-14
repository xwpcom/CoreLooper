#pragma once

#include "net/simpleconnect.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

class HttpPostAckHandler
{
public:
	virtual ~HttpPostAckHandler() {}
	virtual void OnPostAck(const string& ack) = 0;//post发送成功，并且收到完整的回复时调用本接口
	virtual void OnPostFail(int error, string desc) {}//post发送失败
};

//XiongWanPing 2018.07.06
//用http post数据和文件到服务器
class HTTP_EXPORT HttpPost :public SimpleConnect
{
	SUPER(SimpleConnect)
public:
	HttpPost();
	~HttpPost();

	void SetBoundary(const string& boundary)
	{
		mBoundary = boundary;
	}
	void SetAckHandler(shared_ptr<HttpPostAckHandler> handler)
	{
		mPostAckHandler = handler;
	}
	void SetServerPort(string server, int port = 80);
	void AddHeader(string name, string value);
	void AddField(string name, string value);

	//filePath必须为存在的文件
	int AddFile(string name, string filePath);
	void SetBodyRawData(const ByteBuffer& box);
	void SetBody(const string& text);
	int Start(string url);
protected:
	void OnConnect(Channel* endPoint, long error, ByteBuffer* box, Bundle* extraInfo);

	string	mServer;//ip or domain
	int		mPort = 80;
	string	mUrl;//要以/开头,示例 /hzndms/api/alarm/send

	map<string, string>  mHeaders;

	struct tagItem
	{
		string name;
		string value;
		long   mBytes = 0;//仅mFiles有意义
	};
	vector<tagItem>  mFields;
	vector<tagItem>  mFiles;

	void ParseInbox();
	void OnRecvHttpAckDone();

	enum eSendStage
	{
#undef ITEM
#define ITEM(x)	x
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

	const char* GetStageDesc(eSendStage v);

	int  PrepareData();

#ifdef _DEBUG
	int  PackData();
#endif

	void PreStage(eSendStage stage);
	void PostStage(eSendStage stage);
	void SwitchStage(eSendStage stage);
	virtual void OnOutboxWritable();

	struct tagSendInfo
	{
		eSendStage mStage = eSendNone;
		shared_ptr<FILE> mFile;
		int mFileIndex = -1;//正在发送的文件下标
	}mInfo;

	string mBoundary;
	bool mRecvAckDone = false;
	shared_ptr<HttpPostAckHandler> mPostAckHandler;

	bool IsPostFileFields()const
	{
		return (mFiles.size() > 0 || mFields.size() > 0);
	}

	ByteBuffer mBodyRawData;
};

}
}
}
}
