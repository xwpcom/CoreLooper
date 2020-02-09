#include "pch.h"
#include "logmanager.h"

LogManager::LogManager()
{

}

void LogManager::OnCreate()
{

}

void LogManager::OnLogItemReady(shared_ptr<LogItem> item)
{
	bool changed = false;
	{
		auto& apps = mApps;
		auto iter = apps.find(item->appName);
		if (iter == apps.end())
		{
			apps[item->appName] = true;
			changed = true;
		}
	}

	{
		auto& apps = mTags;
		auto iter = apps.find(item->tag);
		if (iter == apps.end())
		{
			apps[item->tag] = true;
			changed = true;
		}
	}

	if (changed)
	{
		SaveConfig();
	}

}

void LogManager::SaveConfig()
{

}
