#pragma once

namespace Bear {
namespace Core
{
namespace FileSystem{

struct tagIniKey
{
#define INI_KEY_COMMENT	0x00
#define INI_KEY_VALUE	0x01

	tagIniKey()
	{
		m_nKeyType = INI_KEY_COMMENT;
	}

	virtual ~tagIniKey()
	{
	}

	int			m_nKeyType;
	std::string	m_szKeyName;
	std::string	m_szKeyValue;
};

struct tagIniSection
{
	std::string mSectionName;

	virtual ~tagIniSection()
	{
	}

	std::list<std::shared_ptr<tagIniKey>> mKeys;
};

#include "thread/criticalsection.h"
#include "base/bytebuffer.h"

//XiongWanPing 2009.12.12
//仿win32 ini格式配置文件
class CORE_EXPORT IniFile
{
public:
	IniFile();
	virtual ~IniFile();

	virtual int Load(const char *pszIniFile);
	virtual int Save();

	virtual int Load(ByteBuffer& box);
	virtual int Dump(ByteBuffer& box);
	virtual int Dump(const std::string& filePath);

	void Empty();
	std::string GetFilePath()
	{
		return mFilePath;
	}

#ifdef _DEBUG
	static int Test();
#endif

	DWORD	GetVersion()const
	{
		return mVersion;
	}

	ULONGLONG GetLastModifyTick()const
	{
		return mTickChanged;
	}

	bool IsModified()const
	{
		return mTickChanged != 0;
	}

	bool GetBool(const char *pszSection, const char *pszName, bool defaultValue = false);
	int SetBool(const char *pszSection, const char *pszName, bool nValue)
	{
		return SetInt(pszSection, pszName, nValue);
	}
	int GetInt(const char *pszSection, const char *pszName, int nDefaultValue = 0);
	int GetInt(std::string pszSection, const char *pszName, int nDefaultValue = 0)
	{
		return GetInt(pszSection.c_str(), pszName, nDefaultValue);
	}
	int SetInt(const char *pszSection, const char *pszName, int nValue);
	int SetInt(std::string section, const char *pszName, int nValue)
	{
		return SetInt(section.c_str(), pszName, nValue);
	}
	std::string GetString(const char *pszSection, const char *pszKey, const char *pszDefault = "");
	std::string GetString(std::string section, const char *pszKey, const char *pszDefault = "")
	{
		return GetString(section.c_str(), pszKey, pszDefault);
	}
	int SetString(const char *pszSection, const char *pszName, const char *pszValue);
	int SetString(std::string section, const char *pszName, const char *pszValue)
	{
		return SetString(section.c_str(), pszName, pszValue);
	}
	std::vector<std::string> GetSectionKeys(const char *pszSection);

protected:
	std::shared_ptr<tagIniSection> FindSection(const char *pszSection, BOOL bCreateIfNoFind = TRUE);
	std::shared_ptr<tagIniKey> FindKey(const char *pszSection, const char *pszKey, BOOL bCreateIfNoFind = TRUE);
	void UpdateLastModifyTick();

	CriticalSection	mCS;
	std::string			mFilePath;
	ULONGLONG			mTickChanged;//最后修改时间
	DWORD				mVersion;//每次数据有变化都增加版本号
	std::list<std::shared_ptr<tagIniSection>> mSections;
};
}
}}
