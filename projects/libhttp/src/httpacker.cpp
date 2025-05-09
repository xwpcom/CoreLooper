﻿#include "stdafx.h"
#include "httpacker.h"
#include "string/stringparam.h"
#include "string/utf8tool.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "HttpAcker";

HttpAcker::HttpAcker()
{
}

void HttpAcker::clear()
{
	mHttpVersion.clear();
	mHttpAckCode = 0;
	mFields.clear();
	mBody.clear();
}

int HttpAcker::Parse(const string& ack, bool onlyHeader)
{
	clear();

	auto posFirstLine = ack.find("\r\n");
	if (posFirstLine == string::npos)
	{
		static int idx = 0;
		if (idx < 10)
		{
			++idx;
			LogV(TAG, "invalid http ack");
		}
		//ASSERT(FALSE);
		return -1;
	}

	auto pos = ack.find(' ');
	if (pos < posFirstLine)
	{
		mHttpVersion = ack.substr(0, pos);
		mHttpAckCode = atoi(ack.c_str() + pos + 1);
	}
	else
	{
		static int idx = 0;
		if (idx < 10)
		{
			++idx;
			LogW(TAG, "invalid http ack");
		}

		//ASSERT(FALSE);
		return -1;
	}

	auto headerTail = ack.find("\r\n\r\n", posFirstLine + 2);// 2 is "\r\n"
	if (headerTail != string::npos)
	{
		auto begin = ack.c_str() + posFirstLine + 2;
		auto end = ack.c_str() + headerTail;
		auto p = begin;
		while (p < end)
		{
			auto off = p - ack.c_str();
			auto lineEnd = ack.find("\r\n", off);
			if (lineEnd > headerTail)
			{
				break;
			}

			//p[0,lineEnd]
			string line=ack.substr(off,lineEnd-off);
			auto dot = line.find(':');
			if (dot != string::npos)
			{
				string name = line.substr(0, dot);
				string value=line.substr(dot+1);
				StringTool::Trim(name, ' ');
				StringTool::Trim(value, ' ');
				mFields[name] = value;
			}

			p += (lineEnd - off) + 2;
		}
	}

	if (onlyHeader)
	{
		return 0;
	}

	{
		auto key = "Content-Length";
		auto it = mFields.find(key);
		if (it != mFields.end())
		{
			auto bytes = atoll(mFields[key].c_str());
			if (bytes > 0)
			{
				auto start = headerTail + 4;
				if (ack.length() < start + bytes)
				{
					return -1;
				}

				mBody = string(ack.c_str() + start, (size_t)bytes);
			}

		}
	}

	{
		//todo:增加对Transfer-Encoding: chunked的处理

		auto it = mFields.find("Transfer-Encoding");
		if (it == mFields.end())
		{
			it= mFields.find("transfer-encoding");
		}

		if (it != mFields.end())
		{
			if (it->second.find("chunked") != string::npos)
			{
				auto start = headerTail + 4;
				auto p = ack.c_str() + start;
				
				HttpChunker obj;
				mBody = obj.Input(p);
			}
		}

	}
	#ifdef _MSC_VER
	mBody = Utf8Tool::escapeUnicode(mBody);
	#endif
	return 0;
}

string HttpChunker::Input(const string& text)
{
	auto sz = text.c_str();
	auto szEnd = sz + text.length();
	string ack;
	while (1)
	{
		auto lineEnd = strstr(sz, "\r\n");
		if (!lineEnd)
		{
			break;
		}
		lineEnd += 2;//2 is \r\n

		char* end = nullptr;
		auto bytes = strtol(sz, &end, 16);
		if (bytes <= 0)
		{
			break;
		}

		if (lineEnd + bytes > szEnd)
		{
			LogW(TAG, "invalid length");
			break;
		}

		ack += string(lineEnd, bytes);

		sz = lineEnd + bytes + 2;
	}

	return ack;
}

}
}
}
}

