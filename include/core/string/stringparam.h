#pragma once

namespace Bear {
namespace Core
{

//XiongWanPing 2018.11.16
//解析处理一行文本，格式name1=value1,name2=value2,
class CORE_EXPORT StringParam
{
public:
	static map<string, string> ParseItems(const string& items, const char *itemSeperator = ",", const char *sign = "=",bool trim=false);
	static unordered_map<string, string> ParseItemsEx(const string& items, const char* itemSeperator = ",", const char* sign = "=");
	static string MergeFields(const string& src, const string& fields, const char *itemSeperator = ",", const char *sign = "=");
	static string ToString(const map<string, string>& items, const char* itemSeperator = ",", const char* sign = "=");
	//value1,value2,value3,no name and "="
	static string MergeFieldsSimple(const string& src, const string& fields, const char *itemSeperator = ",");
};
}
}
