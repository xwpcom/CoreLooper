#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2016.09.03
//解析http ack header
//已过时，建议使用HttpAcker
class HTTP_EXPORT HttpAckParser
{
public:
	HttpAckParser(std::string  sz = "");
	virtual ~HttpAckParser();

	void Parse(const std::string & sz);

	struct tagHttpAckInfo
	{
		tagHttpAckInfo()
		{
			Reset();
		}

		void Reset()
		{
			mAckCode = 0;
			mBody.clear();
		}

		int mAckCode;
		string mBody;
	};


	const tagHttpAckInfo& GetAckInfo()const
	{
		return mInfo;
	}
	const string& httpBody() {
		return mInfo.mBody;
	}
	const string& body() {
		return mInfo.mBody;
	}


protected:
	tagHttpAckInfo mInfo;
};
}
}
}
}