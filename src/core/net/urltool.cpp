#include "stdafx.h"
#include "urltool.h"

using namespace std;

namespace Bear {
namespace Core {
namespace Net {

string UrlTool::GetUriTitle(string url)
{
	const char *key = "//";
	auto pos = (int)url.find(key);

	if (pos != -1)
	{
		url = url.substr(pos + strlen(key));

		//eat host
		pos = (int)url.find("/");
		if (pos != -1)
		{
			url = url.substr(pos + 1);
		}
	}


	pos = (int)url.find("?");
	if (pos != -1)
	{
		url = url.substr(0,pos);
	}

	pos = (int)url.rfind('.');
	if (pos != -1)
	{
		url = url.substr(0,pos);
	}

	return url;
}

string UrlTool::PercentHexString(const string& text)
{
	string ack;
	auto len = text.length();
	for (size_t i = 0; i < len; i++)
	{
		StringTool::AppendFormat(ack,"%%%02X", (BYTE)text[i]);
	}

	return ack.c_str();
}

#if 0
string UrlTool::GetUriTitle(string url)
{
	const char *key = "http://";
	if (url.Find(key) == 0)
	{
		int pos = url.Find("/", strlen(key) + 1);
		if (pos != -1)
		{
			url = url.Right(url.length() - pos);
		}
	}

	int pos = url.Find('.');
	if (pos != -1)
	{
		int end = pos;// +4;//不包括后面的.xml

		int start = 0;
		char ch = url.GetAt(0);
		if (ch == '/' || ch == '\\')
		{
			start = 1;
		}

		return url.Mid(start, end - start);
	}

	return "";
}
#endif

char UrlTool::CharToInt(char ch)
{
	if (ch >= '0' && ch <= '9')return (char)(ch - '0');
	if (ch >= 'a' && ch <= 'f')return (char)(ch - 'a' + 10);
	if (ch >= 'A' && ch <= 'F')return (char)(ch - 'A' + 10);
	return -1;
}

char UrlTool::StrToBin(const char* str)
{
	char tempWord[2];
	char chn;
	tempWord[0] = CharToInt(str[0]); //make the B to 11 -- 00001011 
	tempWord[1] = CharToInt(str[1]); //make the 0 to 0 -- 00000000 
	chn = (tempWord[0] << 4) | tempWord[1]; //to change the BO to 10110000 
	return chn;
}

// from zlmediakit
string UrlTool::UrlEncode(const string& str)
{
	string out;
	size_t len = str.size();
	for (size_t i = 0; i < len; ++i) {
		char ch = str[i];
		if (isalnum((uint8_t)ch) || ch == '-') {
			out.push_back(ch);
		}
		else {
			char buf[4];
			sprintf(buf, "%%%X%X", (uint8_t)ch >> 4, (uint8_t)ch & 0x0F);
			out.append(buf);
		}
	}
	return out;
}

}
}
}
