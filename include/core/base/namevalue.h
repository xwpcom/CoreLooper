#pragma once
#include<unordered_map>

namespace Bear {
namespace Core
{
using namespace std;
struct tagNameValue
{
	std::string name;
	std::string value;
};

//XiongWanPing 2013.01.25
//管理name-value字段
class CORE_EXPORT NameValue
{
public:
	NameValue();
	virtual ~NameValue();

	int Set(const char* key, const char* value);
	int Set(const char* key, int value);
	int Set(const char* key, const string& value);
	int Set(const string& key, const string& value);
	int Set(const string& key, int value);

	string GetString(const string& key, const string& defaultValue = "")const;
	int GetInt(const string& key, int defaultValue = 0)const;
	bool IsKeyExists(const char* key)const;
	bool IsExists(const char* key)const
	{
		return IsKeyExists(key);
	}
	float GetFloat(const string& key, float defaultValue = 0.0f)const;
	bool GetBool(const string& key, bool defaultValue = false)const;

	void clear();
	bool empty()const
	{
		return mItems.empty();
	}

	const unordered_map<string, string>& GetItemsX()const//为方便枚举使用,不要直接修改返回的CListEx
	{
		return mItems;
	}

	void Append(const NameValue& src);
	void Dump()const;
	void RemoveKey(const string& key);

protected:
	unordered_map<string, string> mItems;
};

}
}