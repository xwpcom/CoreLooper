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
	template <class T> void Set(std::string name, T tmp)
	{
		std::stringstream ss;
		ss << tmp;
		Set(name, ss.str());
	}

	void Set(const std::string& name, const std::string& value)
	{
		mItems[name] = value;
	}

	bool IsKeyExists(std::string name)const
	{
		auto iter = mItems.find(name);
		return iter != mItems.end();
	}

	bool GetBool(const std::string&name, bool defaultValue = false)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		return atoi(GetString(name).c_str()) != 0;
	}

	int GetInt(const std::string&name, int defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		return atoi(GetString(name).c_str());
	}

	long GetLong(const std::string&name, long defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		return atol(GetString(name).c_str());
	}

	double GetDouble(const std::string&name, double defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		return atof(GetString(name).c_str());
	}

	LONGLONG GetLongLong(const std::string&name, LONGLONG defaultValue = 0)const
	{
		if (!IsKeyExists(name))
		{
			return defaultValue;
		}

		return atoll(GetString(name).c_str());
	}

	const std::string GetString(const std::string&name, const char *defaultValue = "")const
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

	void Remove(const std::string& name)
	{
		auto iter = mItems.find(name);
		if (iter != mItems.end())
		{
			mItems.erase(iter->first);
		}
	}

	std::string Pack()const
	{
		std::string items;
		for (auto iter = mItems.begin(); iter != mItems.end(); ++iter)
		{
			items += StringTool::Format("%s=%s\r\n", iter->first.c_str(), iter->second.c_str());
		}
		return std::move(items);
	}

public:
	std::map<std::string, std::string> mItems;
};
}
}