#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2020.08.06
//http ack parser,是对HttpAckParser的重构

class HTTP_EXPORT HttpAcker
{
public:
	HttpAcker();

	//ack必须是完整的http回复
	int Parse(const string& ack);

	void clear();

	const string& version()const
	{
		return mHttpVersion;
	}

	int code()const
	{
		return mHttpAckCode;
	}

	unordered_map<string, string>& fields()
	{
		return mFields;
	}

	const string& body()const
	{
		return mBody;
	}

protected:
	string mHttpVersion;//http/1.1
	int mHttpAckCode = 0;//200,404,...
	unordered_map<string, string> mFields;
	string mBody;
};

}
}
}
}
