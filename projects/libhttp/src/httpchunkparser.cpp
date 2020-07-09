#include "stdafx.h"
#include "httpchunkparser.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpChunkParser::HttpChunkParser()
{

}

HttpChunkParser::~HttpChunkParser()
{

}

//httpAck应该是一个完整的http request ack
int HttpChunkParser::Parse(const string& httpAck, ByteBuffer& box)
{
	box.MakeSureEndWithNull();

	auto& sz = httpAck;
	const char* headerTailKey = "\r\n\r\n";

	auto pos = sz.find(headerTailKey);
	if (pos == string::npos)
	{
		return -1;
	}

	{
		const char* key = "Transfer-Encoding: chunked";
		auto posChunked = sz.find(key);
		if (posChunked == string::npos || posChunked > pos)
		{
			return -1;
		}
	}

	//get length hex string
	char* end = nullptr;
	auto bytes = strtol(sz.c_str() + pos + strlen(headerTailKey), &end, 16);
	//DV("bytes=%d", bytes);

	if (bytes <= 0)
	{
		return -1;
	}
	//DV("end=[%s]", end);

	auto body = end + strlen("\r\n");
	box.Write(body, bytes);
	box.MakeSureEndWithNull();
	return 0;
}

}
}
}
}
