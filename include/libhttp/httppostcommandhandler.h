﻿#pragma once
#include "httpformfield.h"
#include "httpheader.h"
#include "libhttp/httpconfig.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {
class HttpRequest;
//XiongWanPing 2016.09.09
//处理http post command基类,抽象出一个框架，由子类处理具体的命令
class HttpPostCommandHandler
{
public:
	HttpPostCommandHandler();
	virtual ~HttpPostCommandHandler();

	virtual int Init(shared_ptr<HttpHeader> header);
	virtual void SetConfig(shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}
	void setHttpRequest(HttpRequest* obj) {
		mHttpRequest = obj;
	}
	void setAck(const string& ack)
	{
		mAck = ack;
	}

	virtual string GetAck();

	//接收完所有数据时会调用本接口
	//error为0表示成功，否则表示失败
	virtual void OnFinishRecvData(int error);
	virtual HttpFormField::eResult Input(ByteBuffer& inbox);

	virtual int BeginField(const string & fieldName, int rangeStart = 0);
	virtual int EndField();

	void SetContentLength(int v)
	{
		mContentLength = v;
	}
protected:
	virtual shared_ptr<HttpFormField> CreateField(const string & fieldName);

protected:
	HttpRequest* mHttpRequest = nullptr;
	shared_ptr<HttpHeader> mHeader;

	shared_ptr<HttpFormField> mCurrentField;
	vector<shared_ptr<HttpFormField>> mFields;
	shared_ptr<tagWebServerConfig> mWebConfig;

	int mContentLength = 0;
	ByteBuffer mInboxBody;
	string mAck;
};
}
}
}
}