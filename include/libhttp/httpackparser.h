#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2016.09.03
//解析http ack header
class HttpAckParser
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
		}

		int mAckCode;
	};


	const tagHttpAckInfo& GetAckInfo()const
	{
		return mInfo;
	}
protected:
	tagHttpAckInfo mInfo;
};
}
}
}
}