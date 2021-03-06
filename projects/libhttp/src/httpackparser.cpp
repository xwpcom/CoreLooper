﻿#include "stdafx.h"
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

//支持Content-Length和Transfer-Encoding: chunked两种格式

void HttpAckParser::Parse(const string & sz)
{
	mInfo.Reset();

	{
		int pos = sz.find(' ');
		if (pos)
		{
			mInfo.mAckCode = atoi(sz.c_str() + pos + 1);
		}
	}

	auto& ack = sz;
	auto key = "Content-Length: ";/* 2021.06.02有些http server不讲武德，没带空格(也是符合rfc规范,是我这里没完善)  */
	auto pos = ack.find(key);
	if (pos == -1)
	{
		key = "Content-Length:";
		pos = ack.find(key);
	}

	if (pos == -1)
	{
		const char* headerTailKey = "\r\n\r\n";
		auto pos = sz.find(headerTailKey);
		if (pos == string::npos)
		{
			return ;
		}

		{
			const char* key = "Transfer-Encoding: chunked";
			auto posChunked = sz.find(key);
			if (posChunked == string::npos || posChunked > pos)
			{
				{
					auto ps = ack.c_str() + pos+strlen(headerTailKey);
					mInfo.mBody = string(ps);
				}
				
				return ;
			}
		}

		//get length hex string
		char* end = nullptr;
		auto bytes = strtol(sz.c_str() + pos + strlen(headerTailKey), &end, 16);
		//DV("bytes=%d", bytes);

		if (bytes <= 0)
		{
			return ;
		}
		//DV("end=[%s]", end);

		auto body = end + strlen("\r\n");
		mInfo.mBody=string(body, bytes);
		return;
	}

	pos += strlen(key);
	auto bytes = atoi(ack.c_str() + pos);
	if (bytes <= 0)
	{
		return;
	}

	key = "\r\n\r\n";
	pos = ack.find(key);
	if (pos == -1)
	{
		return;
	}
	pos += strlen(key);

	ByteBuffer box;
	box.Write((LPBYTE)ack.c_str() + pos, bytes);
	box.MakeSureEndWithNull();

	mInfo.mBody = (char*)box.data();
}
}
}
}
}