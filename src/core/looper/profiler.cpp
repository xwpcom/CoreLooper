#include "stdafx.h"
#include "profiler.h"

namespace Bear {
namespace Core {

#if defined _CONFIG_PROFILER
tagProfiler::tagProfiler()
{
	
}


#endif

Profiler::Profiler(const string& name, int minTick)
{
#if defined _CONFIG_PROFILER
	mName = name;
	mMinTick = minTick;
	mStartTick = ShellTool::GetTickCount64();
#endif
}

Profiler::~Profiler()
{
#if defined _CONFIG_PROFILER
	if (Looper::CurrentLooper()->profilerEnabled() && mStartTick>0)
	{
		auto obj = Looper::CurrentLooper()->profiler();
		obj->fields[mName].times++;

		auto tick = ShellTool::GetTickCount64() - mStartTick;
		if (tick >= mMinTick)
		{
			if (tick > obj->fields[mName].maxTick)
			{
				obj->fields[mName].maxTick = tick;
			}
		}
	}
#endif
}

}
}
