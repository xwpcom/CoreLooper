#include "stdafx.h"
#include "tickdumper.h"
namespace Bear {
namespace Core
{
DWORD TickDumper::mDefaultAlarmDuration = 30;

#ifndef IDS_TRACKER
#define IDS_TRACKER		""
#endif

TickDumper::TickDumper(const char *comment, bool dump, DWORD tickAlarmDuration)
{
	//DT("%s#enter",mComment);
	mComment = comment;
	mEnableDump = dump;
	mTickAlarmDuration = tickAlarmDuration ? tickAlarmDuration : mDefaultAlarmDuration;
	mTickStart = ShellTool::GetTickCount64();

	if (mEnableDump)
	{
		DT("%s%s#begin", IDS_TRACKER, mComment.c_str());
	}
}

TickDumper::~TickDumper()
{
	//DT("%s#exit",mComment);
	if (mEnableDump)
	{
		auto gap = ShellTool::GetTickCount64() - mTickStart;
		DT("%s%s#end,tick=" FMT_LONGLONG, IDS_TRACKER, mComment.c_str(), gap);
	}
	if (mTickAlarmDuration)
	{
		auto gap = ShellTool::GetTickCount64() - mTickStart;
		if (gap > mTickAlarmDuration)
		{
			DW("%s%s timeout,gap=" FMT_LONGLONG ",mTickAlarmDuration=%lu", IDS_TRACKER, mComment.c_str(), gap, mTickAlarmDuration);
			if (Looper::IsMainLooper(Looper::CurrentLooper()))
			{
				//int x = 0;
			}
			//ASSERT(FALSE);
		}
	}
}

void TickDumper::SetDefaultAlarmDuration(DWORD duration)
{
	mDefaultAlarmDuration = duration;
}
}
}