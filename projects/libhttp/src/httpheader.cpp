#include "stdafx.h"
#include "httpheader.h"
#include "string/textseparator.h"
#include "httptool.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{

}

int HttpHeader::ParseStartLine(const string & line)
{
	//样本: POST /upgrade.cgi HTTP/1.1
	//没有考虑空白符采用TAB的情况
	auto pos = line.find(" ");
	if (pos != -1)
	{
		mInfo.mHttpMethod = line.substr(0, pos);

		auto pos2 = line.rfind(' ');
		if (pos2 != -1 && pos2 > pos)
		{
			mInfo.mUrl = line.substr(pos + 1, pos2 - pos - 1);

			HttpTool::ParseUrlParam(mInfo.mUrl, mInfo.mUri, mInfo.mUrlParams);
			StringTool::Replace(mInfo.mUri, "/", "");
		}
	}

	//DV("http method=[%s]", mInfo.mHttpMethod.c_str());
	//DV("http url   =[%s]", mInfo.mUrl.c_str());
	return 0;
}

int HttpHeader::ParseLine(const string & line)
{
	//格式为name: value
	auto pos = line.find(':');
	if (pos != -1)
	{
		string  name = line.substr(0, pos);
		string  value = StringTool::Right(line, (long)(line.length() - pos - 1));
		StringTool::TrimLeft(value, ' ');

		mInfo.mFields.Set(name, value);

		//DV("[%s]=[%s]", name.c_str(), value.c_str());
		if (name == "Content-Type")
		{
			const char *key = "boundary=";
			//Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryUxnYqUOngu5937wp
			auto pos = value.find(key);
			if (pos != -1)
			{
				string  boundary = StringTool::Right(value, (long)(value.length() - pos - strlen(key)));
				auto posEnd = boundary.find(';');
				if (posEnd != -1)
				{
					boundary = boundary.substr(0, posEnd);
				}

				mInfo.mFields.Set("boundary", boundary);
			}

		}
	}

	return 0;
}

int HttpHeader::Parse(const string & ackHeader)
{
	mInfo.Reset();

	int lineIndex = -1;
	TextSeparator lineParser(ackHeader.c_str(), "\r\n");
	while (1)
	{
		string  line;
		int ret = lineParser.GetNext(line);
		if (ret)
		{
			break;
		}

		++lineIndex;

		if (lineIndex == 0)
		{
			ParseStartLine(line);
		}
		else
		{
			ParseLine(line);
		}
	}

	int x = 0;
	return 0;
}

}
}
}
}