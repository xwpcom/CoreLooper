#pragma once
namespace Bear {
namespace Core
{
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

	int Set(const char *key, const char *value);
	int Set(const char *key, std::string value);
	int Set(const char *key, int value);
	int Set(std::string key, std::string value);
	int Set(std::string key, int value);

	std::string GetString(std::string key, std::string defaultValue = "")const;
	int GetInt(std::string key, int defaultValue = 0)const;
	bool IsKeyExists(const std::string&key)const
	{
		return IsKeyExists(key.c_str());
	}
	bool IsKeyExists(const char *key)const;
	bool IsExists(const char *key)const
	{
		return IsKeyExists(key);
	}
	float GetFloat(std::string key, float defaultValue = 0.0f)const;
	bool GetBool(std::string key, bool defaultValue = false)const;

	const tagNameValue *GetItem(const std::string& key)const
	{
		return GetItem(key.c_str());
	}
	const tagNameValue *GetItem(const char *key)const;
	tagNameValue *GetItemEx(const std::string& key, BOOL createAlways)
	{
		return GetItemEx(key.c_str(), createAlways);
	}
	tagNameValue *GetItemEx(const char *key, BOOL createAlways);
	const tagNameValue *GetExistsItem(const char *key)const;
	void Empty();

	int GetVersion()const
	{
		return mVersion;
	}

	const std::list<tagNameValue>&  GetItems()const//为方便枚举使用,不要直接修改返回的CListEx
	{
		return mItems;
	}

	void Append(const NameValue& src);
	void Dump();

private:
	NameValue & operator=(NameValue& src);
	NameValue(const NameValue&);
protected:
	void SetModified();
protected:
	std::list<tagNameValue> mItems;
	int			mVersion;//版本管理，用来检测键值是否变化
};

}
}