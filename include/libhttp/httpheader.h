#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

struct tagHttpHeader
{
	tagHttpHeader()
	{
		Reset();
	}

	void Reset()
	{
		mHttpMethod.clear();
		mAckCode = 0;
		mUrl.clear();
		mUri.clear();
	}

	std::string 	mHttpMethod;
	int			mAckCode;//200,404...
	std::string 	mUrl;//完整的url,比如/xx.cgi?user=admin&password.admin
	std::string 	mUri;//只包括主名称,比如xx.cgi

	NameValue	mUrlParams;
	NameValue	mFields;
};

//XiongWanPing 2016.09.08
//解析http头数据
class HttpHeader
{
public:
	HttpHeader();
	virtual ~HttpHeader();

	int Parse(const std::string & text);
	tagHttpHeader& GetHeader()
	{
		return mInfo;
	}

protected:
	int ParseStartLine(const std::string & line);
	int ParseLine(const std::string & line);

	tagHttpHeader mInfo;
};
}
}
}
}