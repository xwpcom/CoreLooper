#pragma once
namespace Bear {
namespace Core
{
enum eTextFlag
{
	eTextFlag_None = 0,
	eTextFlag_TrimTail = 0x0001,//清除结尾的'\r'字符
};
//#include "std::string/StringEx2.h"
//XiongWanPing 2016.03.31
class CORE_EXPORT TextSeparator
{
public:
	TextSeparator(const char *text, const char *delimiter = "\r\n");
	TextSeparator(const string& text, const char* delimiter = "\r\n");
	int GetNext(std::string& sz, eTextFlag flag = eTextFlag_None);
	static void Parse(const std::string& text, const std::string& delimiter, std::vector<std::string>& items);
	/*
	int GetNext(StringEx& sz, eTextFlag flag = eTextFlag_None)
	{
		std::string item;
		int ret=GetNext(item,flag);
		sz = item.c_str();
		return ret;
	}
	//*/
protected:
	const char *mBegin = nullptr;
	const char *mDelimiter = nullptr;
};
}
}