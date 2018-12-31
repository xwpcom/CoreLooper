#include "stdafx.h"
#include "handlerex.h"
#include "file/inifile.h"
#include "message.inl"
#include "handlerinternaldata.h"

#ifdef _CONFIG_ANDROID
extern "C"
{
#include <android/log.h>

}
#endif

using namespace std;

namespace Bear {
namespace Core
{
using namespace FileSystem;

CriticalSection	HandlerEx::mLogConfigCS;
map<string, tagObjectLogConfig> HandlerEx::sLogMap;

HandlerEx::HandlerEx()
{
}

int HandlerEx::OnProcDataGetter(const string& name, string& desc)
{
	DW("请在子类实现 %s.%s%s", GetObjectName().c_str(), name.c_str(), desc.c_str());
	ASSERT(FALSE);
	return -1;
}

//test
//int mAge = 4;
//string mName;
int HandlerEx::OnProcDataSetter(string name, int value)
{
	UNUSED(name);
	UNUSED(value);

	/*
	if (name == "age")
	{
		if (value >= 0 && value <= 100)
		{
			mAge = value;
			return 0;
		}
		else
		{
			DW("invalid value %d for %s", value, "age");
		}
	}
	*/

	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , bool )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , BYTE )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string name, DWORD value)
{
	DW("name=%s,value=%d", name.c_str(), value);
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , WORD )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , double )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , LONGLONG )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , ULONGLONG )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::OnProcDataSetter(string , string )
{
	ASSERT(FALSE);
	return eSetterResult_InvalidName;
}

int HandlerEx::Test()
{
#ifdef _DEBUG
	//BindProcData(mAge, "age", "baby age", PDF_READ | PDF_SETTER);
	//BindProcData(mName, "name", "xwp", PDF_READ | PDF_SETTER);

	mProcNode->SetInt("age", 35);
	int value = mProcNode->GetInt("age");
	mProcNode->SetInt("age", 123);
	value = mProcNode->GetInt("age");

	string name = mProcNode->GetString("name");
	mProcNode->SetString("name", "xxy");
	name = mProcNode->GetString("name");

	double height = 50.0;
	BindProcData(height, "height", "height", PDF_READ_WRITE);
	mProcNode->SetDouble("height", 173.0);
	double xx = mProcNode->GetDouble("height");

	DWORD money = 123456789;
	BindProcData(money, "money", "", PDF_READ_WRITE);
	mProcNode->SetDword("money", 1354417884);
	DWORD mm = mProcNode->GetDword("money");

	string xml;
	DumpProcData(xml);
	DV("xml=[%s]", xml.c_str());

#endif
	return 0;
}

void HandlerEx::DumpProcData(string& xmlAck)
{
	bool debug = false;
#ifdef _DEBUG
	//发现CPhoneConnectSvr在proc.xml中不出现,仅在android下出现
	//查找好久，原来是TcpServer_Linux::OnMessage中写了Handler::OnMessage,改用__super::OnMessage即可
	//是提取HandlerEx时遗漏了修改此处
	//坑爹,强烈建议所有.cpp中都用__super来调用父类
	//debug = (GetObjectName() == "CPhoneConnectSvr");
#endif

	if (!IsMyselfThread())
	{
		if (debug)
		{
			DV("%s skip %s due to NOT local looper", GetObjectName().c_str(), __func__);
		}

		return;
	}

	string xml;

	string name = GetObjectName();
	if (name.empty())
	{
		name = "_";
	}
	char ch = name[0];
	if (isdigit(ch))
	{
		name = "_" + name;//xml字段名字不能以数字开头
	}

	xml += StringTool::Format("<%s>", name.c_str());
	bool hasData = false;
	{
		if (debug)
		{
			if (mProcNode)
			{
				DV("mProcNode is exists");
			}
			else
			{
				DV("mProcNode is null");
			}
		}

		if (mProcNode)
		{
			mProcNode->DumpData(xml);
			hasData = true;
		}

		{
			string xmlChild;
			auto& items = mInternalData->mChildren;
			for (auto iter = items.begin(); iter != items.end(); ++iter)
			{
				auto item = iter->second.lock();
				if (item && !item->MaybeLongBlock())
				{
					//DV("item=[%s]", item->GetObjectName().c_str());
					item->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xmlChild);//item有可能位于其他线程
				}
			}

			if (xmlChild.empty())
			{
				if (debug)
				{
					DV("xmlChild is empty");
				}
			}
			else
			{
				if (debug)
				{
					DV("xmlChild=%s", xmlChild.c_str());
				}
				xml += StringTool::Format("<%s>%s</%s>", CHILD_NODE_NAME, xmlChild.c_str(), CHILD_NODE_NAME);

				hasData = true;
			}
		}
	}

	if (hasData)
	{
		xml += StringTool::Format("</%s>", name.c_str());
		xmlAck += xml;
	}
}

void HandlerEx::LogV(const char* pszFormat, ...)
{
	LOG_IMPL(GetObjectName(), eLogV);
}

void HandlerEx::LogD(const char* pszFormat, ...)
{
	LOG_IMPL(GetObjectName(), eLogD);
}

void HandlerEx::LogI(const char* pszFormat, ...)
{
	LOG_IMPL(GetObjectName(), eLogI);
}

void HandlerEx::LogW(const char* pszFormat, ...)
{
	LOG_IMPL(GetObjectName(), eLogW);
}

void HandlerEx::LogE(const char* pszFormat, ...)
{
	LOG_IMPL(GetObjectName(), eLogE);
}

void HandlerEx::Log(const string& tag, int level, tagObjectLogConfig& cfg, const char* text)
{
	{
		//直接在此logcat
		if (level >= cfg.mDumpLevel)
		{
#if defined _CONFIG_ANDROID
			__android_log_print((android_LogPriority)level, ("Bear/" + tag).c_str(), "%s", text);
#else
			if (level == eLogV)
			{
				DV("%s", text);
			}
			else if (level == eLogI)
			{
				DG("%s", text);
			}
			else if (level == eLogW)
			{
				DW("%s", text);
			}
			else if (level == eLogE)
			{
				DE("%s", text);
			}
			else if (level == eLogA)
			{
				DE("%s", text);
				ASSERT(FALSE);
			}

			//printf("%s\n", text);
#endif
		}
	}

	if (level >= cfg.mDumpFileLevel)
	{
		//保存文件要发给main looper来串行处理

		auto obj = Looper::GetMainLooper();
		if (obj)
		{
			tagLogItem *item = new tagLogItem;
			item->mLevel = level;
			item->mTag = tag;
			item->mText = text;
			item->mThreadId = ShellTool::GetCurrentThreadId();

			if (Looper::CurrentLooper() == obj)
			{
				obj->sendMessage(BM_ADD_FILE_LOG_ITEM, (WPARAM)item);
			}
			else
			{
				obj->postMessage(BM_ADD_FILE_LOG_ITEM, (WPARAM)item);
			}
		}
		else
		{
			DW("no main looper,skip log(%s)", text);
		}
	}
}

void HandlerEx::GetLogConfig(const string& objName, tagObjectLogConfig& cfg)
{
	AutoLock lock(&mLogConfigCS);
	auto iter = sLogMap.find(objName);
	if (iter != sLogMap.end())
	{
		cfg = iter->second;
	}
	else
	{
		tagObjectLogConfig def;
		sLogMap[objName] = def;
		cfg = def;
	}
}

void HandlerEx::GetLogMap(map<string, tagObjectLogConfig>& obj)
{
	AutoLock lock(&mLogConfigCS);
	for (auto iter = sLogMap.begin(); iter != sLogMap.end(); ++iter)
	{
		obj[iter->first] = iter->second;
	}
}

void HandlerEx::SetTagConfig(const string& tags, int dumpLevel, int dumpFileLevel)
{
	//多个tag以逗号分隔
	TextSeparator demux(tags.c_str(), ",");
	vector<string> vec;
	string item;
	while (demux.GetNext(item) == 0)
	{
		if (!item.empty())
		{
			vec.push_back(item.c_str());
		}
	}

	AutoLock lock(&mLogConfigCS);
	if (dumpLevel != -1)
	{
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			sLogMap[*iter].mDumpLevel = dumpLevel;
		}

	}

	if (dumpFileLevel != -1)
	{
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			sLogMap[*iter].mDumpFileLevel = dumpFileLevel;
		}
	}
}

int HandlerEx::GetLogLevel(char ch)
{
	static char levels[] = { '.','.','V','D','I','W','E','A', };
	for (size_t i = 0; i < sizeof(levels); ++i)
	{
		if (ch == levels[i])
		{
			if (i >= eLogMin && i <= eLogMax)
			{
				return (int)i;
			}
			break;
		}
	}

	return eLogV;
}

void HandlerEx::LoadLogConfig(IniFile *ini, const string& section)
{
	vector<string> vec = ini->GetSectionKeys(section.c_str());
	vector<tagObjectLogConfig> configs;
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		tagObjectLogConfig cfg;
		string value = ini->GetString(section.c_str(), iter->c_str());
		int len = (int)value.length();
		if (len > 0)
		{
			cfg.mDumpLevel = GetLogLevel(value[0]);
		}
		if (len > 1)
		{
			cfg.mDumpFileLevel = GetLogLevel(value[1]);
		}

		configs.push_back(cfg);
	}

	ASSERT(vec.size() == configs.size());

	{
		AutoLock lock(&mLogConfigCS);
		int idx = -1;
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			++idx;
			sLogMap[iter->c_str()] = configs[idx];
		}
	}
}

static bool SortString(const string &v1, const string &v2)
{
	return v1.compare(v2) < 0;
}

void HandlerEx::SaveLogConfig(IniFile *ini, const string& section)
{
	map<string, tagObjectLogConfig> items;
	GetLogMap(items);

	//按名称排序
	vector<string> vec;
	for (auto iter = items.begin(); iter != items.end(); ++iter)
	{
		vec.push_back(iter->first);
	}

	std::sort(vec.begin(), vec.end(), SortString);

	static char levels[] = { '.','.','V','D','I','W','E','A', };

	string value;
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		auto cfg = items[*iter];
		value = StringTool::Format("%c%c", levels[cfg.mDumpLevel], levels[cfg.mDumpFileLevel]);
		ini->SetString(section.c_str(), (*iter).c_str(), value.c_str());
	}
}

LRESULT HandlerEx::OnMessage(UINT msg,WPARAM wp,LPARAM lp)
{
	switch (msg)
	{
	case BM_ADD_FILE_LOG_ITEM:
	{
		//这里是避免main looper没响应此消息时，用来自动删除item
		tagLogItem *item = (tagLogItem *)wp;
		DW("should handle in main looper,tag=[%s],text=[%s]",item->mTag.c_str(),item->mText.c_str());
		delete item;
		item = nullptr;

		return 0;
	}
	case BM_DUMP_PROC_DATA:
	{
		string* xml = (string*)wp;
		if (xml)
		{
			DumpProcData(*xml);
		}
		else
		{
			ASSERT(FALSE);
		}
		return 0;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

}
}
