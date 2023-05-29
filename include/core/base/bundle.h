#pragma once
#include <map>
#include <string>
#include <sstream>
#include <iostream>
namespace Bear {
namespace Core
{
//XiongWanPing 2017.09.23
class Bundle
{
public:
	template <class T> void Set(string name, T tmp)
	{
		stringstream ss;
		ss << tmp;
		Set(name, ss.str());
	}

	void Set(const string& name, const string& value)
	{
		mItems[name] = value;
	}

	bool IsKeyExists(string name)const
	{
		auto iter = mItems.find(name);
		return iter != mItems.end();
	}

	bool GetBool(const string&name, bool defaultValue = false)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		auto v = GetString(name);
		if (v.empty())
		{
			return defaultValue;
		}
		return atoi(v.c_str()) != 0;
	}

	int GetInt(const string&name, int defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		auto v = GetString(name);
		if (v.empty())
		{
			return defaultValue;
		}
		return atoi(v.c_str());
	}

	long GetLong(const string&name, long defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		auto v = GetString(name);
		if (v.empty())
		{
			return defaultValue;
		}
		return atol(v.c_str());
	}

	double GetDouble(const string&name, double defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		auto v = GetString(name);
		if (v.empty())
		{
			return defaultValue;
		}
		return atof(v.c_str());
	}

	LONGLONG GetLongLong(const string&name, LONGLONG defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		auto v = GetString(name);
		if (v.empty())
		{
			return defaultValue;
		}
		return atoll(v.c_str());
	}

	const string GetString(const string&name, const string& defaultValue="")const
	{
		auto iter = mItems.find(name);
		if (iter != mItems.end())
		{
			return iter->second;
		}

		return defaultValue;
	}

	void clear()
	{
		mItems.clear();
	}

	void Remove(const string& name)
	{
		auto iter = mItems.find(name);
		if (iter != mItems.end())
		{
			mItems.erase(iter->first);
		}
	}

	string Pack()const
	{
		string items;
		for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
		{
			items += StringTool::Format("%s=%s\r\n", iter->first.c_str(), iter->second.c_str());
		}
		return items;
	}

public:
	map<string, string> mItems;
};
}
}