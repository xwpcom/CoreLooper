#include "stdafx.h"
#include "base/namevalue.h"
#include "base/stringtool.h"
using namespace std;
using namespace Bear::Core;

NameValue::NameValue()
{
	mVersion = 0;
}

NameValue::~NameValue()
{
	Empty();
}

void NameValue::Empty()
{
	if(mItems.size()>0)
	{
		SetModified();
	}

	mItems.clear();
}

void NameValue::SetModified()
{
	mVersion++;
}

int NameValue::Set(const char *key, const char *value)
{
	int ret = -1;

	BOOL createAlways = TRUE;
	tagNameValue* item = GetItemEx(key, createAlways);
	if (item)
	{
		if (item->value != value)
		{
			item->value = value;
			SetModified();
		}

		ret = 0;
	}

	return ret;
}

int NameValue::Set(const char *key, string value)
{
	return Set(key, value.c_str());
}

int NameValue::Set(string key, string value)
{
	int ret = -1;

	BOOL createAlways = TRUE;
	tagNameValue* item = GetItemEx(key, createAlways);
	if (item)
	{
		if (item->value != value)
		{
			item->value = value;
			SetModified();
		}
		ret = 0;
	}

	return ret;
}

int NameValue::Set(string key, int value)
{
	string szValue=StringTool::Format("%d", value);
	return Set(key, szValue);
}

int NameValue::Set(const char *key, int value)
{
	string szValue=StringTool::Format("%d", value);
	return Set(key, szValue);
}

float NameValue::GetFloat(string key, float defaultValue)const
{
	if (!IsKeyExists(key))
	{
		return defaultValue;
	}

	return (float)atof(GetString(key).c_str());
}

bool NameValue::GetBool(string key, bool defaultValue)const
{
	if (!IsKeyExists(key))
	{
		return defaultValue;
	}

	string value = GetString(key);
	if (value.empty())
	{
		return defaultValue;
	}

	if (StringTool::CompareNoCase(value,"0")==0 || StringTool::CompareNoCase(value,"false") == 0)
	{
		return false;
	}

	return true;
}

string NameValue::GetString(string key, string defaultValue)const
{
	if (!IsKeyExists(key))
	{
		return defaultValue;
	}

	const tagNameValue* item = GetItem(key);
	if (item)
	{
		return item->value;
	}

	return defaultValue;
}

int NameValue::GetInt(string key, int defaultValue)const
{
	if (!IsKeyExists(key))
	{
		return defaultValue;
	}

	const tagNameValue* item = GetItem(key);
	if (item)
	{
		return atoi(item->value.c_str());
	}

	return defaultValue;
}

const tagNameValue* NameValue::GetItem(const char *key)const
{
	for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
	{
		if (StringTool::CompareNoCase(iter->name,key) == 0)
		{
			return &(*iter);
		}
	}

	return nullptr;
}

const tagNameValue *NameValue::GetExistsItem(const char *key)const
{
	for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
	{
		if (StringTool::CompareNoCase(iter->name,key) == 0)
		{
			return &(*iter);
		}
	}

	return nullptr;
}

tagNameValue* NameValue::GetItemEx(const char *key, BOOL createAlways)
{
	for (int i = 0; i < 2; i++)
	{
		for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
		{
			if (StringTool::CompareNoCase(iter->name,key) == 0)
			{
				return &(*iter);
			}
		}

		//ASSERT(i == 0);

		if (createAlways)
		{
			tagNameValue item;
			item.name = key;
			mItems.push_back(item);
			SetModified();
		}
	}

	return NULL;
}

bool NameValue::IsKeyExists(const char *key)const
{
	const tagNameValue* item = GetExistsItem(key);
	return item != NULL;
}

void NameValue::Append(const NameValue& src)
{
	for (auto iter = src.mItems.begin(); iter != src.mItems.end(); ++iter)
	{
		Set(iter->name, iter->value);
	}
}

void NameValue::Dump()
{
	int idx = -1;
	for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
	{
		++idx;
		DV("%02d: [%s] = [%s]", idx, iter->name.c_str(), iter->value.c_str());
	}
}
