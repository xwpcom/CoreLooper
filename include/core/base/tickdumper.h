#pragma once
namespace Bear {
namespace Core
{
//XiongWanPing 2012.07.05
class CORE_EXPORT TickDumper
{
public:
	TickDumper(const char *comment, bool dump = false, DWORD tickAlarmDuration = 0);
	~TickDumper();

	static void SetDefaultAlarmDuration(DWORD duration);
protected:
	std::string mComment;

	ULONGLONG	mTickAlarmDuration;	//超时报警
	ULONGLONG	mTickStart;			//开始时间
	bool		mEnableDump = true;

	static DWORD	mDefaultAlarmDuration;	//默认超时报警
};
}
}