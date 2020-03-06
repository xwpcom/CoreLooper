#pragma once

namespace Bear {
namespace Core {
namespace FileSystem {
//XiongWanPing 2020.03.05
/*
Ini是IniFile的高性能简化版,特点如下:
.支持section,key,value
.不支持注释,注释可采用section,key,value来表示
.不支持保持字段顺序,便于采用unordered_map
.不做同步,约定在单线程中使用
*/

//有些环境下只支持map,不支持unordered_map,比如sim868上面
//#define _map  map
#define _map  unordered_map 

typedef _map<string, string> IniSection;

class CORE_EXPORT Ini
{
public:
	Ini();
	virtual ~Ini();

	//为提高性能，外部可直接只读访问,比如读取ini[section][name],但修改要使用Set(),否则版本和自动保存机制不能工作
	//重载[]后有个缺点:没法追踪字段修改,不方便做自动保存,
	IniSection& operator[](const string& section)
	{
		return mSections[section];
	}

	void Set(const string&section, const string& name, int value);
	void Set(const string& section, const string& name, bool value);
	void Set(const string& section, const string& name, double value);
	void Set(const string& section, const string& name, const string& value);
	void Set(const string& section, const string& name, const char* value)
	{
		if (value && value[0])
		{
			Set(section, name, (string )value);
		}
	}

	int GetInt(const string& section, const string& name, int defaultValue=0);
	bool GetBool(const string& section, const string& name, bool defaultValue = false);
	double GetDouble(const string& section, const string& name, double defaultValue = 0.0f);
	string GetString(const string& section, const string& name, string defaultValue = "");

	long version()const
	{
		return mVersion;
	}
	
	bool IsModified()const
	{
		return mModified;
	}

	void Load(ByteBuffer& box);
	void Dump(ByteBuffer& box);
	
	static int Load(Ini& ini,const string& filePath);
	static int Save(Ini& ini, const string& filePath);

	void RemoveSection(const string& section);
	void RemoveKey(const string& section, const string& key);

	void clear();
	void clearModifiedFlag()
	{
		mModified = false;
		mVersion = 0;
		mTickModified = 0;
	}

protected:
	_map<string, IniSection> mSections;

	void SetModified();
	bool mModified = false;

	long mVersion = 0;//每次修改时版本号递增
	ULONGLONG			mTickModified=0;//最后修改时间
};

}
}
}
