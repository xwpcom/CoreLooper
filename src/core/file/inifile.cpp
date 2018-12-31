#include "stdafx.h"
#include "inifile.h"
#include "base/dt.h"
#include "base/stringtool.h"
#include "string/textseparator.h"
#include "file/file.h"
using namespace std;
namespace Bear {
namespace Core
{
namespace FileSystem {
IniFile::IniFile()
{
	mTickChanged = 0;
	mVersion = 0;
}

IniFile::~IniFile()
{
	Empty();
}

//从box中加载数据
int IniFile::Load(ByteBuffer& box)
{
	AutoLock ac(&mCS);
	Empty();

	const int len = box.GetActualDataLength();
	if (len == 0)
	{
		return 0;
	}

	const char *sz = (const char*)box.GetDataPointer();
	if (len >= 3)
	{
		//跳过utf-8格式BOM
		//utf-8文件以0xEF,0xBB,0xBF开头
		LPBYTE pHeader = (LPBYTE)sz;
		if (pHeader[0] == 0xEF && pHeader[1] == 0xBB && pHeader[2] == 0xBF)
		{
			sz += 3;
		}
	}

	TextSeparator demux(sz, "\n");

	shared_ptr<tagIniSection> currentSection;
	string line;
	while (demux.GetNext(line, eTextFlag_TrimTail) == 0)
	{
		const char *psz = line.c_str();
		int len = (int)line.length();

		if (len > 2 && psz[0] == '[' && psz[len - 1] == ']')
		{
			auto item = make_shared<tagIniSection>();
			item->mSectionName = line;
			//DV("section[%s]", ps->mSectionName.c_str());
			currentSection = item;
			mSections.push_back(currentSection);
			continue;
		}

		if (!currentSection)
		{
			//构造一个空段
			currentSection = make_shared<tagIniSection>();
			mSections.push_back(currentSection);
		}

		int pos = (int)line.find("=");
		bool comment = (pos == -1);
		if (!comment && len > 0)
		{
			char ch = line[0];
			if (ch == '#' || ch == ';')
			{
				comment = true;
			}
		}

		if (comment)
		{
			auto item = make_shared<tagIniKey>();
			item->m_nKeyType = INI_KEY_COMMENT;
			item->m_szKeyName = line;
			currentSection->mKeys.push_back(item);
			continue;
		}

		//TextSeparator parse(line, "=");
		string key = line.substr(0, pos);
		string value = line.substr(pos + 1);

		auto item = make_shared<tagIniKey>();
		item->m_nKeyType = INI_KEY_VALUE;
		item->m_szKeyName = key;
		item->m_szKeyValue = value;
		currentSection->mKeys.push_back(item);
		//DV("key:[%s]=[%s]",szKey,pszValue);
	}

	mTickChanged = 0;

	return 0;
}

int IniFile::Dump(ByteBuffer& box)
{
	box.clear();

	{
		AutoLock ac(&mCS);

		string content;

		for (auto iter = mSections.begin(); iter != mSections.end(); ++iter)
		{
			auto item = *iter;
			if (!item->mSectionName.empty())
			{
				content += StringTool::Format("%s\n", item->mSectionName.c_str());
			}

			for (auto it = item->mKeys.begin(); it != item->mKeys.end(); ++it)
			{
				auto item = *it;
				if (item->m_nKeyType == INI_KEY_COMMENT)
				{
					content += StringTool::Format("%s\n", item->m_szKeyName.c_str());
				}
				else if (item->m_nKeyType == INI_KEY_VALUE)
				{
					content += StringTool::Format("%s=%s\n", item->m_szKeyName.c_str(), item->m_szKeyValue.c_str());
				}
				else
				{
					ASSERT(FALSE);
				}
			}
		}

		box.PrepareBuf((int)content.length() + 1);
		box.Write(content);
		box.MakeSureEndWithNull();
	}

	return 0;
}

int IniFile::Load(const char *filePath)
{
	AutoLock ac(&mCS);

	Empty();
	mFilePath = filePath;

	ByteBuffer box;
	File::ReadFile(mFilePath.c_str(), box);
	box.MakeSureEndWithNull();

	return Load(box);
}

int IniFile::Dump(const string& filePath)
{
	ByteBuffer box;
	Dump(box);
	return File::Dump(box, filePath.c_str());
}

int IniFile::Save()
{
	if (mFilePath.empty())
	{
		ASSERT(FALSE);
		return -1;
	}

	ByteBuffer box;
	Dump(box);

	int ret = -1;
	{
		File::Dump(box, mFilePath.c_str());
		File::sync();
		ret = 0;
	}

	mTickChanged = 0;
	return ret;
}

void IniFile::Empty()
{
	AutoLock ac(&mCS);
	if (!mSections.empty())
	{
		mSections.clear();
		UpdateLastModifyTick();
	}
}

bool IniFile::GetBool(const char *pszSection, const char *pszName, bool defaultValue)
{
	return !!GetInt(pszSection, pszName, defaultValue);
}

int IniFile::GetInt(const char *pszSection, const char *pszKey, int nDefaultValue)
{
	string value = GetString(pszSection, pszKey, NULL);
	if (!value.empty())
	{
		int ret = atoi(value.c_str());
		return ret;
	}

	return nDefaultValue;
}

int IniFile::SetInt(const char *pszSection, const char *pszName, int value)
{
	string text = StringTool::Format("%d", value);
	int ret = SetString(pszSection, pszName, text.c_str());
	return ret;
}

int IniFile::SetString(const char *pszSection, const char *pszName, const char *pszValue)
{
	AutoLock ac(&mCS);

	auto pk = FindKey(pszSection, pszName, true);
	if (pk && pk->m_szKeyValue != pszValue)
	{
		pk->m_szKeyValue = pszValue;
		pk->m_nKeyType = INI_KEY_VALUE;
		UpdateLastModifyTick();
	}

	return 0;
}

string IniFile::GetString(const char *pszSection, const char *pszKey, const char *pszDefault)
{
	if (!pszDefault)
	{
		pszDefault = "";
	}

	AutoLock ac(&mCS);
	auto pk = FindKey(pszSection, pszKey, FALSE);
	if (!pk)
	{
		//DV("no find %s.%s,return default value(%s)",pszSection,pszKey,pszDefault);
		return pszDefault;
	}

	return pk->m_szKeyValue;
}

//注意返回的指针不是线程安全的
shared_ptr<tagIniSection> IniFile::FindSection(const char *pszSection, BOOL bCreateIfNoFind)
{
	AutoLock ac(&mCS);
	if (!pszSection)
	{
		ASSERT(pszSection);
		return nullptr;
	}

	string szSection;
	if (strlen(pszSection) == 0)
		szSection = "";
	else
		szSection = "[" + string(pszSection) + "]";

	for (auto iter = mSections.begin(); iter != mSections.end(); ++iter)
	{
		if (StringTool::CompareNoCase((*iter)->mSectionName, szSection) == 0)
		{
			return *iter;
		}
	}

	if (bCreateIfNoFind)
	{
		auto item = make_shared<tagIniSection>();
		item->mSectionName = szSection;
		mSections.push_back(item);
		UpdateLastModifyTick();
		return item;
	}

	return nullptr;
}

shared_ptr<tagIniKey> IniFile::FindKey(const char *pszSection, const char *pszKey, BOOL bCreateIfNoFind)
{
	AutoLock ac(&mCS);
	auto ps = FindSection(pszSection, bCreateIfNoFind);

	if (!ps)
	{
		return NULL;
	}

	for (auto iter = ps->mKeys.begin(); iter != ps->mKeys.end(); ++iter)
	{
		if (StringTool::CompareNoCase((*iter)->m_szKeyName, pszKey) == 0)
		{
			return *iter;
		}
	}

	if (bCreateIfNoFind)
	{
		auto item = make_shared<tagIniKey>();
		item->m_szKeyName = pszKey;
		ps->mKeys.push_back(item);
		UpdateLastModifyTick();
		return item;
	}

	return nullptr;
}

void IniFile::UpdateLastModifyTick()
{
	mTickChanged = ShellTool::GetTickCount64();
	mVersion++;
}

vector<string> IniFile::GetSectionKeys(const char *pszSection)
{
	AutoLock ac(&mCS);
	vector<string> vec;
	bool bCreateIfNoFind = false;
	auto ps = FindSection(pszSection, bCreateIfNoFind);
	if (ps)
	{
		for (auto iter = ps->mKeys.begin(); iter != ps->mKeys.end(); ++iter)
		{
			if ((*iter)->m_nKeyType == INI_KEY_VALUE)
			{
				vec.push_back((*iter)->m_szKeyName.c_str());
			}
		}
	}

	return vec;
}

#ifdef _DEBUG
int IniFile::Test()
{
	return 0;
}
#endif
}
}
}
