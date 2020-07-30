#include "stdafx.h"
#include "base/namevalue.h"
#include "base/stringtool.h"
using namespace std;
using namespace Bear::Core;

static const char* TAG = "NameValue";

NameValue::NameValue()
{
}

NameValue::~NameValue()
{
	clear();
}

void NameValue::clear()
{
	mItems.clear();
}

int NameValue::Set(const char* key, const char* value)
{
	if (!key || !key[0] || !value)
	{
		return -1;
	}

	mItems[key] = value;
	return 0;
}

int NameValue::Set(const string& key, int value)
{
	auto v = StringTool::Format("%d", value);
	return Set(key, v);
}

int NameValue::Set(const char* key, const string& value)
{
	mItems[key] = value;
	return 0;
}

int NameValue::Set(const string& key, const string& value)
{
	mItems[key] = value;
	return 0;
}

int NameValue::Set(const char* key, int value)
{
	auto  szValue = StringTool::Format("%d", value);
	return Set(key, szValue);
}

float NameValue::GetFloat(const string& key, float defaultValue)const
{
	auto it = mItems.find(key.c_str());
	if (it == mItems.end())
	{
		return defaultValue;
	}

	return (float)atof(it->second.c_str());
}

bool NameValue::GetBool(const string& key, bool defaultValue)const
{
	auto it = mItems.find(key.c_str());
	if (it == mItems.end())
	{
		return defaultValue;
	}

	auto& v = it->second;
	if (v == "0" || v == "false")
	{
		return false;
	}

	return true;
}

string NameValue::GetString(const string& key, const string& defaultValue)const
{
	auto it = mItems.find(key.c_str());
	if (it == mItems.end())
	{
		return defaultValue;
	}

	return it->second.c_str();
}

int NameValue::GetInt(const string& key, int defaultValue)const
{
	auto it = mItems.find(key.c_str());
	if (it == mItems.end())
	{
		return defaultValue;
	}

	return atoi(it->second.c_str());
}

bool NameValue::IsKeyExists(const char* key)const
{
	if (!key || !key[0])
	{
		return false;
	}

	auto it = mItems.find(key);
	return it != mItems.end();
}

void NameValue::RemoveKey(const string& key)
{
	auto it = mItems.find(key);
	if (it != mItems.end())
	{
		mItems.erase(it);
	}
}

void NameValue::Append(const NameValue& src)
{
	for (auto iter = src.mItems.begin(); iter != src.mItems.end(); ++iter)
	{
		Set(iter->first, iter->second);
	}
}

void NameValue::Dump()const
{
	int idx = -1;
	for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
	{
		++idx;
		LogV(TAG, "%02d: [%s] = [%s]", idx, iter->first.c_str(), iter->second.c_str());
	}
}
