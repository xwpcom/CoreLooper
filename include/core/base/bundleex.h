#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2015.03.17
//可以复制
//BundleEx bundle2=bundle;
class BundleEx
{
public:
	//用宏扩展实现各种类型
	/*
	void PutInt(std::string name, int value)
	{
	mIntItems[name] = value;
	}
	bool IsIntExists(std::string name)
	{
	std::map<std::string, int>::iterator iter = mIntItems.find(name);
	return iter != mIntItems.end();
	}
	int GetInt(std::string name,int defaultValue=0)
	{
	if (IsIntExists(name))
	{
	return mIntItems[name];
	}

	return defaultValue;
	}
	protected:
	std::map<std::string,int> mIntItems;
	*/
#undef ITEM
#define ITEM(type,Type,DefaultValue)		\
public:										\
	void Put##Type(std::string name, type value)	\
	{										\
		m##Type##Items[name] = value;		\
	}										\
	bool Is##Type##Exists(std::string name)									\
	{																\
		std::map<std::string, type>::iterator iter = m##Type##Items.find(name);\
		return iter != m##Type##Items.end();								\
	}																\
	type Get##Type(std::string name, type defaultValue=DefaultValue)		\
	{																\
		if (Is##Type##Exists(name))									\
		{															\
			return m##Type##Items[name];							\
		}															\
																	\
		return defaultValue;										\
	}																\
protected:									\
	std::map<std::string, type> m##Type##Items;	\

	ITEM(int, Int, 0)
		ITEM(long, Long, 0)
		ITEM(bool, Bool, 0)
		ITEM(BOOL, BOOL, 0)
		ITEM(ULONGLONG, ULONGLONG, 0)
		ITEM(float, Float, 0)
		ITEM(double, Double, 0)
		ITEM(std::string, String, "")
		ITEM(HANDLE, Handle, 0)
#undef ITEM
};
}
}