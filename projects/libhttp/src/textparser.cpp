#include "stdafx.h"
#include "textparser.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

TextParser::TextParser()
{

}


#if 0
HTTP和RTSP协议格式基本一致
请求首行格式 : COMMAND url protocol / version
	回复首行格式 : protocol / version ackCode desc

	HTTP------------------------------------------------------------------------------------------
	GET / proc.xml ? user = admin & password = admin & url = IotServer HTTP / 1.1
	Host : 4g.jjyip.com
	Connection : keep - alive
	Upgrade - Insecure - Requests : 1
	User - Agent : Mozilla / 5.0 (Windows NT 6.1; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 74.0.3729.169 Safari / 537.36
	Accept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, image / apng, ; q = 0.8, application / signed - exchange; v = b3
	Purpose : prefetch
	Accept - Encoding : gzip, deflate
	Accept - Language : en, zh - CN; q = 0.9, zh; q = 0.8

	HTTP / 1.1 200 OK
	Content - Type: text / xml; charset = UTF - 8
	Cache - Control: no - cache, no - store
	Content - Length : 12900

	RTSP------------------------------------------------------------------------------------------
	C->S : OPTIONS rtsp ://server.example.com RTSP/2.0
CSeq: 1
	User - Agent : PhonyClient / 1.2
	Proxy - Require : gzipped - messages
	Supported : play.basic

	S->C : RTSP / 2.0 200 OK
	CSeq : 1
	Public : DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, OPTIONS
	Supported : play.basic, setup.rtp.rtcp.mux, play.scale
	Server : PhonyServer / 1.1

#endif

	void TextParser::Parse(const string & input)
{
	clear();
	mText = input;

	string text;
	auto pos = input.find("\r\n\r\n");
	if (pos != -1 && pos != input.length() - 4)
	{
		text = input.substr(0, pos);
		mBody = input.substr(pos + 4);
	}
	else
	{
		text = input;
	}

	TextSeparator obj(text.c_str(), "\r\n");

	int index = -1;
	string line;
	while (obj.GetNext(line) == 0)
	{
		++index;

		if (index == 0)
		{
			//请求首行格式:COMMAND url protocol/version
			//回复首行格式:protocol/version ackCode desc

			TextSeparator obj(line.c_str(), " ");
			string v0, v1, v2;
			obj.GetNext(v0);
			obj.GetNext(v1);
			obj.GetNext(v2);

			if (v0.find("/") != -1)
			{
				//是回复
				mProtocolAndVersion = v0.c_str();
				mAckCode = atoi(v1.c_str());
			}
			else
			{
				//是请求
				mCommand = v0.c_str();
				mUrl = v1.c_str();
				mProtocolAndVersion = v2.c_str();
			}

			continue;
		}

		//行格式:
		//Server: PhonyServer/1.1
		auto pos = line.find(':');
		if (pos != -1)
		{
			string item=line.substr(pos+1);

			auto name = line.substr(0, pos);
			if (mBundle.IsKeyExists(name))
			{
				name += "2";
			}
			mBundle.Set(name, StringTool::Trim(item,' '));
		}

	}

}

}
}
}
}