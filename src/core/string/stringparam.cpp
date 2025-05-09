﻿#include "stdafx.h"
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

string StringParam::MergeFields(const string& base, const string& newFields, const char *itemSeperator, const char *sign)
{
	map<string, string> items1 = ParseItems(base);
	map<string, string> items2 = ParseItems(newFields);

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

void StringParam::ParseItems(const string& items, vector<tagNameValue>& ackItems, const char* itemSeperator, const char* sign)
{
	ackItems.clear();

	TextSeparator obj2(items.c_str(), itemSeperator);
	string nv;
	while (1)
	{
		int ret = obj2.GetNext(nv);
		if (ret)
		{
			break;
		}

		tagNameValue item;
		auto pos = nv.find(sign);
		if (pos != string::npos)
		{
			item.name = nv.substr(0, pos);
			item.value = nv.substr(pos + 1);
		}
		else
		{
			item.name = nv;
		}

		//if (trim)
		{
			StringTool::Trim(item.name);
			StringTool::Trim(item.value);
		}


		ackItems.push_back(item);
	}
}

static const char* TAG = "stringParam";
static int pushItems(tagNameValue* ackItems, int itemCount, int& index, const char* name, const char* value)
{
	if (index < itemCount - 1)
	{
		ackItems[index].name = name;
		ackItems[index].value = value;
		++index;
		return 0;
	}

	LogW(TAG, "%s fail,[%s]=[%s]", __func__, name, value);

	return -1;
}

//在vsTest中测试,TEST_METHOD(stringParam)
//data必须为可写,数据格式name1=value1,name2=value2,
//返回解析出的item个数
int StringParam::ParseItemsInPlace(char* data, tagNameValue* ackItems, int itemCount, const char* sepText)
{
	if (!data || !data[0] || !ackItems || itemCount <= 0 || !sepText || !sepText[0])
	{
		return 0;
	}

	int index = 0;
	char* s = data;
	bool eof = false;
	while (*s)
	{
		auto end = strstr(s, sepText);
		if (end)
		{
			*end = 0;
		}
		else
		{
			eof = true;
		}

		auto sep = strchr(s, '=');
		if (sep)
		{
			*sep = 0;

			char* name = s;
			char* value = sep + 1;

			pushItems(ackItems, itemCount, index, name, value);
		}

		if (eof)
		{
			break;
		}
		else
		{
			s = end + 1;
		}
	}

	return index;
}

NVWrapper::NVWrapper(tagNameValue* items, int count)
{
	mItems = items;
	mCount = count;
}

tagNameValue* NVWrapper::findItem(const char* name)
{
	if (!name || !*name)
	{
		return nullptr;
	}

	for (int i = 0; i < mCount; i++)
	{
		if (strcmp(name, mItems[i].name.c_str()) == 0)
		{
			return &mItems[i];
		}
	}

	return nullptr;
}

const char* NVWrapper::getString(const char* name, const char* defaultValue)
{
	auto item = findItem(name);
	if (item)
	{
		return item->value.c_str();
	}

	return defaultValue;
}

bool NVWrapper::exists(const char* name)
{
	return !!findItem(name);
}

bool NVWrapper::getBool(const char* name, bool defaultValue)
{
	auto item = findItem(name);
	if (item)
	{
		return atoi(item->value.c_str()) != 0;
	}

	return defaultValue;
}

int NVWrapper::getInt(const char* name, int defaultValue)
{
	auto item = findItem(name);
	if (item)
	{
		return atoi(item->value.c_str());
	}

	return defaultValue;
}

double NVWrapper::getDouble(const char* name, double defaultValue)
{
	auto item = findItem(name);
	if (item)
	{
		return atof(item->value.c_str());
	}

	return defaultValue;
}

//保证不返回nullptr
const char* NVWrapper::operator[](const char* name)
{
	auto item = findItem(name);
	if (item && item->value.c_str())
	{
		return item->value.c_str();
	}
	return "";
}

}
}