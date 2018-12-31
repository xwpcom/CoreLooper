#pragma once
#include "rtsp_inc.h"
#include "base/namevalue.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {

//XiongWanPing 2016.03.31
//解析RTSP文本协议
class RTSP_CLASS RtspTextParser
{
public:
	RtspTextParser(string text);
	virtual ~RtspTextParser();

	string GetText()const
	{
		return mText;
	}

	void Parse(string text);

	bool IsRequest()const
	{
		return !mParseError && mRequest;
	}
	bool IsAck()const
	{
		return !mParseError && !mRequest;
	}

	string GetRequestUrl();//仅mRequest为true时有效,是request首行的参数,比如DESCRIBE url ...
	string GetRequestUri();
	string GetCSeq()
	{
		//CSeq虽然是int,但实际上可当作字符串,只是用来唯一标识某个rtsp请求
		return mBundle.GetString("CSeq");
	}

	int GetAckCode()const//仅mRequest为false时有效
	{
		return mAckCode;
	}

	Bundle & GetNameValue()
	{
		return mBundle;
	}

	eRtspRequestCommand GetRtspCommand()const
	{
		return mRtspCommand;
	}

	int GetUserPassword(string& user, string& password);

protected:
	eRtspRequestCommand ParseRtspCommand(string& sz);
	void ParseFirstLine(string& sz);

protected:
	string				mText;

	bool					mRequest;//otherwise is ack
	bool					mParseError;
	int						mAckCode;
	eRtspRequestCommand		mRtspCommand;//仅在mRequest为true时有效
	string				mRequestUrl;//url是完整的地址,比如rtsp://user:password@192.168.1.3/h.264/720p.sdp
	//string				mRequestUri;//uri中去掉前缀，账号和host后的部分，比如h.264/720p.sdp

	Bundle	mBundle;
};

}
}
}
}
