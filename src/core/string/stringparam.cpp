#include "stdafx.h"
#include "stringparam.h"
using namespace std;

namespace Bear {
namespace Core
{

//items格式item1,item2,item3,
//每个item格式:name=value
map<string, string> StringParam::ParseItems(const string& items, const char *itemSeperator, const char *sign)
{
	map<string, string> ack;

	TextSeparator obj2(items.c_str(), itemSeperator);
	string pair2;
	while (1)
	{
		int ret = obj2.GetNext(pair2);
		if (ret)
		{
			break;
		}

		{
			TextSeparator parse(pair2.c_str(), sign);
			string name;
			string value;
			parse.GetNext(name);
			parse.GetNext(value);

			//DV("%s=[%s]", name.c_str(), value.c_str());
			ack[name] = value.c_str();
		}
	}

	return ack;
}

string StringParam::MergeFieldsSimple(const string& src, const string& fields, const char *itemSeperator)
{
	map<string, string> items1 = ParseItems(src);
	map<string, string> items2 = ParseItems(fields);

	map<string, string> items;

	for (map<string, string>::iterator iter = items1.begin(); iter != items1.end(); ++iter)
	{
		if (!iter->second.empty())
		{
			items[iter->first] = iter->second;
		}
	}

	for (map<string, string>::iterator iter = items2.begin(); iter != items2.end(); ++iter)
	{
		items[iter->first] = iter->second;
	}

	string ack;
	for (map<string, string>::iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		StringTool::AppendFormat(ack, "%s%s", iter->first.c_str(), itemSeperator);
	}

	return ack;
}

//合并并清除value为空的field
string StringParam::MergeFields(const string& src, const string& fields, const char *itemSeperator, const char *sign)
{
	map<string, string> items1 = ParseItems(src);
	map<string, string> items2 = ParseItems(fields);

	map<string, string> items;

	for (map<string, string>::iterator iter = items1.begin(); iter != items1.end(); ++iter)
	{
		if (!iter->second.empty())
		{
			items[iter->first] = iter->second;
		}
	}

	for (map<string, string>::iterator iter = items2.begin(); iter != items2.end(); ++iter)
	{
		//约定默认为0,所以0不需要保存
		if (iter->second.empty() || iter->second == "0")
		{
			map<string, string>::iterator it = items.find(iter->first);
			if (it != items.end())
			{
				items.erase(it);
			}
		}
		else
		{
			items[iter->first] = iter->second;
		}
	}

	string ack;
	for (map<string, string>::iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		StringTool::AppendFormat(ack, "%s%s%s%s", iter->first.c_str(), sign, iter->second.c_str(), itemSeperator);
	}

	return ack;
}


}
}