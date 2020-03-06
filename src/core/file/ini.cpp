#include "stdafx.h"
#include "ini.h"
#include "base/dt.h"
#include "base/stringtool.h"
#include "string/textseparator.h"
#include "file/file.h"
using namespace std;
namespace Bear {
namespace Core {
namespace FileSystem {

Ini::Ini()
{
}

Ini::~Ini()
{
}

void Ini::SetModified()
{
	++mVersion;
	mModified = true;
}

void Ini::Set(const string& section, const string& name, int value)
{
	Set(section,name,StringFormat("%d", value));
}

void Ini::Set(const string& section, const string& name, bool value)
{
	Set(section, name, StringFormat("%d", (int)value));
}

void Ini::Set(const string& section, const string& name, double value)
{
	Set(section, name, StringFormat("%f", value));
}

void Ini::Set(const string& section, const string& name, const string& value)
{
	if (mSections[section][name] != value)
	{
		mSections[section][name] = value;
		SetModified();
	}
}

#define _GET \
auto it = mSections.find(section);\
if (it == mSections.end())		  \
{								  \
	return defaultValue;		  \
}								  \
								  \
auto it2 = it->second.find(name); \
if (it2 == it->second.end())	  \
{								  \
	return defaultValue;		  \
}

int Ini::GetInt(const string& section, const string& name, int defaultValue)
{
	_GET;
	return atoi(it2->second.c_str());
}

bool Ini::GetBool(const string& section, const string& name, bool defaultValue)
{
	_GET;
	return (bool)atoi(it2->second.c_str());
}

double Ini::GetDouble(const string& section, const string& name, double defaultValue)
{
	_GET;
	return atof(it2->second.c_str());
}

string Ini::GetString(const string& section, const string& name, string defaultValue)
{
	_GET;
	return it2->second;
}

void Ini::RemoveSection(const string& section)
{
	auto it = mSections.find(section);
	if (it != mSections.end())
	{
		mSections.erase(it);
		SetModified();
	}
}

void Ini::RemoveKey(const string& section, const string& key)
{
	auto it = mSections.find(section);
	if (it != mSections.end())
	{
		auto& items = it->second;
		auto it2 = items.find(key);
		if (it2 != items.end())
		{
			items.erase(it2);
			SetModified();
		}
	}
}

void Ini::clear()
{
	mModified = false;
	mVersion = 0;
	mTickModified = 0;
	mSections.clear();
}

void Ini::Load(ByteBuffer& box)
{
	clear();

	const int len = box.length();
	if (len == 0)
	{
		return;
	}

	const char* sz = (const char*)box.GetDataPointer();
	if (len >= 3)
	{
		//跳过utf-8格式BOM
		//utf-8文件以0xEF,0xBB,0xBF开头
		LPBYTE d = (LPBYTE)sz;
		if (d[0] == 0xEF && d[1] == 0xBB && d[2] == 0xBF)
		{
			sz += 3;
		}
	}

	TextSeparator demux(sz, "\n");

	string section;
	string line;
	while (demux.GetNext(line, eTextFlag_TrimTail) == 0)
	{
		const char* psz = line.c_str();
		int len = (int)line.length();

		if (len > 2 && psz[0] == '[' && psz[len - 1] == ']')
		{
			section = line.substr(1, line.length() - 2);
			continue;
		}

		int pos = (int)line.find("=");
		/*
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
			continue;
		}
		*/

		auto& key = line.substr(0, pos);
		auto& value = line.substr(pos + 1);
		mSections[section][key] = value;
	}
}

int Ini::Load(Ini& ini, const string& filePath)
{
	ini.clear();

	if (File::FileExists(filePath))
	{
		ByteBuffer box;
		File::ReadFile(filePath.c_str(), box);
		box.MakeSureEndWithNull();

		ini.Load(box);
		return 0;
	}

	return -1;
}

int Ini::Save(Ini& ini, const string& filePath)
{
	ByteBuffer box;
	ini.Dump(box);

	int ret = -1;
	ret=File::Dump(box, filePath.c_str());
	if (ret == 0)
	{
		File::sync();
	}

	return ret;
}

void Ini::Dump(ByteBuffer& box)
{
	string text;
	for (auto& section : mSections)
	{
		text += "[" + section.first + "]\n";
		
		for (auto& keys : section.second)
		{
			text += keys.first + "=" + keys.second + "\n";
		}
	}

	box.PrepareBuf((int)text.length() + 1);
	box.Write(text);
	box.MakeSureEndWithNull();
}

}
}
}
