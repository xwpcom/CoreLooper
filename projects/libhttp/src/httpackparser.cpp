#include "stdafx.h"
#include "httpackparser.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {
HttpAckParser::HttpAckParser(string  sz)
{
	Parse(sz);
}

HttpAckParser::~HttpAckParser()
{

}

void HttpAckParser::Parse(const string & sz)
{
	mInfo.Reset();

	auto pos = sz.find(" ");
	if (pos)
	{
		mInfo.mAckCode = atoi(sz.c_str() + pos + 1);
	}
}
}
}
}
}