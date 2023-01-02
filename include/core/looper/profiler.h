#pragma once

namespace Bear {
namespace Core {

#if defined _CONFIG_PROFILER
struct tagProfiler
{
	tagProfiler();

	void clear()
	{
		startTick = 0;
		stopTick = 0;
		totalSteps = 0;
		ioCount = 0;
		ioContextMaxTick = 0;
		timerMaxTick = 0;
		timerCallCount = 0;
		msgCount = 0;
		tcpAcceptCount = 0;
		tcpRecvCount = 0;
		tcpSentCount = 0;
		tcpConnectAckCount = 0;
	}

	ULONGLONG startTick = 0;
	ULONGLONG stopTick = 0;
	ULONGLONG totalSteps = 0;
	ULONGLONG ioCount = 0;
	ULONGLONG ioContextMaxTick = 0;//单个IoContext执行的最大tick
	ULONGLONG timerMaxTick = 0;
	ULONGLONG timerCallCount = 0;
	ULONGLONG msgCount = 0;
	ULONGLONG tcpAcceptCount = 0;
	ULONGLONG tcpRecvCount = 0;
	ULONGLONG tcpSentCount = 0;
	ULONGLONG tcpConnectAckCount = 0;

};
#endif

}
}
