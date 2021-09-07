#pragma once
namespace Bear {
namespace Core{
namespace Net {

class CORE_EXPORT UrlTool
{
public:
	static std::string GetUriTitle(std::string url);
	static string PercentHexString(const string& text);
	static string UrlEncode(const string& str);
	static char CharToInt(char ch);
	static char StrToBin(const char* str);
};

}
}
}