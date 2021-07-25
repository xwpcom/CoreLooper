#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

class HTTP_EXPORT HttpChunker
{
public:
	string Input(const string& text);
};

	//text must be chunker data,sample:
	/*
14
,"client_id":"saber"
17
,"account":"sz2_device"
2d
,"jti":"f8ead0ee-d499-46e0-8d24-58294c3e9325"
1
}
0
	*/

//XiongWanPing 2020.08.06
//http ack parser,是对HttpAckParser的重构

class HTTP_EXPORT HttpAcker
{
public:
	HttpAcker();

	//ack必须是完整的http回复或header
	int Parse(const string& ack,bool onlyHeader = false);

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
