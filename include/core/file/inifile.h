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
	string	m_szKeyName;
	string	m_szKeyValue;
};

struct tagIniSection
{
	string mSectionName;

	virtual ~tagIniSection()
	{
	}

	list<shared_ptr<tagIniKey>> mKeys;
};

#include "core/thread/criticalsection.h"
#include "core/base/bytebuffer.h"

//XiongWanPing 2009.12.12
//仿win32 ini格式配置文件
class CORE_EXPORT IniFile
{
public:
	IniFile();
	virtual ~IniFile();

	virtual int Load(const char *pszIniFile);
	virtual int Load(const string& filePath)
	{
		return Load(filePath.c_str());
	}
	virtual int Save();

	virtual int Load(ByteBuffer& box);
	virtual int Dump(ByteBuffer& box);
	virtual int Dump(const string& filePath);

	void Empty();
	string GetFilePath()
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
	int GetInt(string pszSection, const char *pszName, int nDefaultValue = 0)
	{
		return GetInt(pszSection.c_str(), pszName, nDefaultValue);
	}
	int SetInt(const char *pszSection, const char *pszName, int nValue);
	int SetInt(const string& section, const char *pszName, int nValue)
	{
		return SetInt(section.c_str(), pszName, nValue);
	}
	string GetString(const char *pszSection, const char *pszKey, const char *pszDefault = "");
	string GetString(string section, const char *pszKey, const char *pszDefault = "")
	{
		return GetString(section.c_str(), pszKey, pszDefault);
	}

	int SetString(const string& section, const string& name, const string& value)
	{
		return SetString(section.c_str(), name.c_str(), value.c_str());
	}

	string GetString(const string& section, const string& key, const string& defaultValue = "")
	{
		return GetString(section.c_str(), key.c_str(), defaultValue.c_str());
	}

#ifdef _MSC_VER
	CString GetStringMFC(const string& section, const string& pszKey, const string& pszDefault = "")
	{
		USES_CONVERSION;
		return A2T(GetString(section.c_str(), pszKey.c_str(), pszDefault.c_str()).c_str());
	}
	CString GetStringMFC(CString section, CString pszKey, CString pszDefault = _T(""))
	{
		USES_CONVERSION;
		return A2T(GetString(T2A(section), T2A(pszKey), T2A(pszDefault)).c_str());
	}
	CString GetStringMFC(const string& section, CString pszKey, CString pszDefault = _T(""))
	{
		USES_CONVERSION;
		return A2T(GetString(section.c_str(), T2A(pszKey), T2A(pszDefault)).c_str());
	}
	int SetStringMFC(CString section, CString name, CString value)
	{
		USES_CONVERSION;
		return SetString(T2A(section), T2A(name), T2A(value));
	}
	int SetStringMFC(const string& section, const string& name, CString value)
	{
		USES_CONVERSION;
		return SetString(section.c_str(), name.c_str(), T2A(value));
	}
	int SetStringMFC(const string& section, CString name, CString value)
	{
		USES_CONVERSION;
		return SetString(section, T2A(name), T2A(value));
	}
	int SetInt(const string& section, CString name, int value)
	{
		USES_CONVERSION;
		return SetInt(section, T2A(name), value);
	}
	int GetInt(const string& section, const CString& name, int nDefaultValue = 0)
	{
		USES_CONVERSION;
		return GetInt(section.c_str(), T2A(name), nDefaultValue);
	}
#endif
	int SetString(const char *pszSection, const char *pszName, const char *pszValue);
	int SetString(string section, const char *pszName, const char *pszValue)
	{
		return SetString(section.c_str(), pszName, pszValue);
	}
	int SetString(string section, const char* pszName, const string& pszValue)
	{
		return SetString(section.c_str(), pszName, pszValue.c_str());
	}
	vector<string> GetSectionKeys(const char *pszSection);

	//not thread safe
	list<shared_ptr<tagIniSection>>& Sections()
	{
		return mSections;
	}

	void RemoveSection(const string& section);

protected:
	shared_ptr<tagIniSection> FindSection(const char *pszSection, BOOL bCreateIfNoFind = TRUE);
	shared_ptr<tagIniKey> FindKey(const char *pszSection, const char *pszKey, BOOL bCreateIfNoFind = TRUE);
	void UpdateLastModifyTick();

	CriticalSection	mCS;
	string			mFilePath;
	ULONGLONG			mTickChanged;//最后修改时间
	DWORD				mVersion;//每次数据有变化都增加版本号
	list<shared_ptr<tagIniSection>> mSections;
};
}
}}
