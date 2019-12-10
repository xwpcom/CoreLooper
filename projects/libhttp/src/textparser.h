#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Http {
//XiongWanPing 2019.05.30
//用于解析http,rtsp等常见的文本协议字段

class TextParser
{
public:
	TextParser();
	void Parse(const string& text);

	Bundle mBundle;
	string mCommand;
	string mUrl;
	string mProtocolAndVersion;
	int mAckCode = -1;
	string mBody;

	void clear()
	{
		mBundle.clear();
		mCommand.clear();
		mUrl.clear();
		mProtocolAndVersion.clear();
		mAckCode = -1;
		mBody.clear();
	}
};

}
}
}
}
