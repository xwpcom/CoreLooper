#pragma once
#include "loginfo.h"

//XiongWanPing 2020.02.06
class LogManager: public sigslot::has_slots<>
{
public:
	LogManager();
	void OnCreate();
	map<string, bool>& apps()
	{
		return mApps;
	}

	map<string, bool>& tags()
	{
		return mTags;
	}

	void OnLogItemReady(shared_ptr<LogItem> item);
protected:
	map<string,bool>	mApps;//列出目前已知的所有app,bool只占位，无意义
	map<string, bool>	mTags;//列出目前已知的所有app,bool只占位，无意义
	void SaveConfig();

};