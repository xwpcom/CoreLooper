#pragma once

namespace Bear {
namespace Core
{

//XiongWanPing 2018.11.16
//解析处理一行文本，格式name1=value1,name2=value2,
class CORE_EXPORT StringParam
{
public:
	static void ParseItems(const string& items, vector<tagNameValue>& ackItems, const char* itemSeperator = ",", const char* sign = "=");
	static map<string, string> ParseItems(const string& items, const char *itemSeperator = ",", const char *sign = "=",bool trim=false);
	static unordered_map<string, string> ParseItemsEx(const string& items, const char* itemSeperator = ",", const char* sign = "=");
	static string MergeFields(const string& src, const string& fields, const char *itemSeperator = ",", const char *sign = "=");
	static string ToString(const map<string, string>& items, const char* itemSeperator = ",", const char* sign = "=");
	//value1,value2,value3,no name and "="
	static string MergeFieldsSimple(const string& src, const string& fields, const char *itemSeperator = ",");

	//data必须可写
	static int ParseItemsInPlace(char* data, tagNameValue* ackItems, int itemCount, const char* sepText = ",");

};

class CORE_EXPORT NVWrapper
{
public:
	NVWrapper(tagNameValue* items, int count);

	const char* getString(const char* name, const char* defaultValue = "");
	int getInt(const char* name, int defaultValue = 0);
	bool getBool(const char* name, bool defaultValue = false);
	double getDouble(const char* name, double defaultValue = 0);
	const char* operator[](const char* name);
	bool exists(const char* name);
protected:
	tagNameValue* findItem(const char* name);
	tagNameValue* mItems = nullptr;
	int mCount = 0;

};


}
}
