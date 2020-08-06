#include "stdafx.h"
#include "httpacker.h"
#include "string/stringparam.h"

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

/*
HTTP/1.1 200 OK
Date: Thu, 06 Aug 2020 09:57:17 GMT
Server: webserver
X-Content-Type-Options: nosniff
X-Frame-Options: SAMEORIGIN
X-XSS-Protection: 1; mode=block
Content-Length: 230
Connection: keep-alive
Keep-Alive: timeout=8, max=99
Cache-control: no-cache="set-cookie"
Content-Type: application/xml
Set-Cookie: WebSession_0eb6f4251b=2a65bbbaeb553b3a01a5e93c783d8fd2c67406a333eb6ffc83269ddd60843b98; path=/;HttpOnly

<?xml version="1.0" encoding="UTF-8"?>
<SessionLogin>
<statusValue>200</statusValue>
<statusString>OK</statusString>
<isSupportLoginTiming>false</isSupportLoginTiming>
<sessionIDVersion>2</sessionIDVersion>
</SessionLogin>
*/
int HttpAcker::Parse(const string& ack)
{
	clear();

	auto posFirstLine = ack.find("\r\n");
	if (posFirstLine == string::npos)
	{
		LogW(TAG, "invalid http ack");
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
		LogW(TAG, "invalid http ack");
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
			auto dot = ack.find(':', off);
			if (dot != string::npos && dot < headerTail)
			{
				string name(p, dot - off);
				string value(ack.c_str() + dot + 2, lineEnd - dot - 2);
				mFields[name] = value;
			}

			p += (lineEnd - off) + 2;
		}
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
		if (it != mFields.end())
		{
			if (it->second.find("chunked") != string::npos)
			{
				auto start = headerTail + 4;
				auto end = ack.length();
				while (start < end)
				{
					auto p = ack.c_str() + start;
					auto bytes = (int)strtoull(p, nullptr, 16);
					if (bytes == 0)
					{
						break;
					}

					auto lineEnd = ack.find("\r\n", start);
					if (lineEnd == string::npos)
					{
						break;
					}

					if (ack.length() < lineEnd + 2 + bytes)
					{
						LogW(TAG, "invalid http ack");
						return -1;
					}

					mBody += string(ack.c_str() + lineEnd + 2, (size_t)bytes);

					start += bytes + 6;
				}
			}
		}

	}


	return 0;
}

}
}
}
}

