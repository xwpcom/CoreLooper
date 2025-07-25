﻿#pragma once
#include "core/base/win32.h"
#include <stdlib.h>
#include <functional>
#include "core/base/dt.h"

namespace Bear {
namespace Core
{
using namespace std;
template<typename ... Args>
std::string StringFormat(const char* format, Args ... args)
{
	size_t size = snprintf(nullptr, 0, format, args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format, args ...);
	return std::string(buf.get(), buf.get() + size - 1);
};

//XiongWanPing 2018.04.21
class CORE_EXPORT StringTool
{
public:
	static int ByteToHexChar(const unsigned char* pByte, int cbByte, char* dest, int destBytes);
	static int AppendText(char* dst, int dstBytes, const char* text);
	static int AppendFormat(char* dst, int dstBytes, const char* fmt, ...);
	static void split(const char* text, const char* sep, char* itemBuf, size_t itemBufBytes, function<void(const char* item)> fn);

	static std::string xml(std::string sz);
	static std::string Format(const char* lpszFormat, ...);
	static std::string& AppendFormat(std::string& obj, const char* lpszFormat, ...);
	static bool StartWith(const std::string&obj, const std::string& key);
	static bool EndWith(const std::string& obj, const std::string& tail);
	static int CompareNoCase(const std::string&obj1, const std::string& obj2);
	static int stricmp(const std::string&obj1, const std::string& obj2)
	{
		return CompareNoCase(obj1, obj2);
	}
	static std::string& Replace(std::string&obj, const std::string& src, const std::string& target);
	static std::string& Trim(std::string&obj, char ch=' ');
	static std::string& TrimLeft(std::string&obj, char ch = ' ');
	static std::string& TrimRight(std::string&obj, char ch = ' ');
	static std::string& MakeUpper(std::string&obj);
	static std::string& MakeLower(std::string&obj);
	static void makeHexPretty(string& hex);
	static bool IsHexChar(char ch);
	static bool IsHexString(const char *sz);

	static int ExtractSubString(const std::string& src, char chSep, std::vector<std::string>& arrString)
	{
		return ExtractSubString(src.c_str(),chSep, arrString);
	}
	static int ExtractSubString(const char *pszSource, char chSep, std::vector<std::string>& arrString);
	static int ExtractInts(const string& src, char chSep, std::vector<int>& items);
	static const char *stristr(const char *psz0, const char *psz1);
	static const char *stristr(const std::string& sz0, const std::string& sz1)
	{
		return stristr(sz0.c_str(), sz1.c_str());
	}
	static std::string Right(const std::string& obj, long bytes);

#ifdef _DEBUG
	static int Test();
#endif
};
}
}