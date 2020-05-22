#pragma once
#include "httpformfield.h"
#include "httpheader.h"
#include "libhttp/httpconfig.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2016.09.09
//处理http post command基类,抽象出一个框架，由子类处理具体的命令
class HttpPostCommandHandler
{
public:
	HttpPostCommandHandler();
	virtual ~HttpPostCommandHandler();

	int Init(shared_ptr<HttpHeader> header);
	virtual void SetConfig(shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}

	//接收完所有数据时会调用本接口
	//error为0表示成功，否则表示失败
	virtual void OnFinishRecvData(int error);
	HttpFormField::eResult Input(ByteBuffer& inbox);

	int BeginField(const string & fieldName, int rangeStart = 0);
	int EndField();
protected:
	virtual shared_ptr<HttpFormField> CreateField(const string & fieldName);

protected:
	shared_ptr<HttpHeader> mHeader;

	shared_ptr<HttpFormField> mCurrentField;
	vector<shared_ptr<HttpFormField>> mFields;
	shared_ptr<tagWebServerConfig> mWebConfig;

};
}
}
}
}