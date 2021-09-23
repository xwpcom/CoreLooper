#include "stdafx.h"
#include "stringparam.h"
using namespace std;

namespace Bear {
namespace Core
{

unordered_map<string, string> StringParam::ParseItemsEx(const string& items, const char* itemSeperator, const char* sign)
{
	unordered_map<string, string> ack;

	TextSeparator obj2(items.c_str(), itemSeperator);
	string nv;
	while (1)
	{
		int ret = obj2.GetNext(nv);
		if (ret)
		{
			break;
		}

		string name, value;
		auto pos = nv.find(sign);
		if (pos != string::npos)
		{
			name = nv.substr(0, pos);
			value = nv.substr(pos + 1);
		}
		else
		{
			name = nv;
		}

		ack[name] = value;
	}

	return ack;
}

//items格式item1,item2,item3,
//每个item格式:name=value
map<string, string> StringParam::ParseItems(const string& items, const char *itemSeperator, const char *sign, bool trim)
{
	map<string, string> ack;

	TextSeparator obj2(items.c_str(), itemSeperator);
	string nv;
	while (1)
	{
		int ret = obj2.GetNext(nv);
		if (ret)
		{
			break;
		}

		string name, value;
		auto pos = nv.find(sign);
		if(pos!=string::npos)
		{
			name = nv.substr(0, pos);
			value= nv.substr(pos+1);
		}
		else
		{
			name = nv;
		}

		if (trim)
		{
			StringTool::Trim(name);
			StringTool::Trim(value);
		}

		ack[name] = value;
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

string StringParam::ToString(const map<string, string>& items, const char* itemSeperator, const char* sign)
{
	string ack;
	for (auto& item : items)
	{
		ack += item.first + sign + item.second + itemSeperator;
	}
	return ack;
}

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
		//约定默认为0,所以0不需要保存 */
		//2021.09.03 0或空也要保存并传给设备，否则当设备上配置有数据时，没法清为0

		/*
		if (iter->second.empty())// || iter->second == "0")
		{
			map<string, string>::iterator it = items.find(iter->first);
			if (it != items.end())
			{
				items.erase(it);
			}
		}
		else
		*/
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