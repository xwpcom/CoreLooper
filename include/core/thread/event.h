#pragma once

#ifdef _MSC_VER
#define WEBRTC_WIN
#else
#define WEBRTC_POSIX
#endif

namespace Bear {
namespace Core
{
//XiongWanPing 2015.05.09
//copy from webrtc
//G:\work\opensource\webrtc\webrtc\base\event.cc
class CORE_EXPORT Event
{
public:
	static const int kForever = -1;

	Event(bool manual_reset=true, bool initially_signaled=false);
	~Event();

	void Set();
	void Reset();

	// Wait for the event to become signaled, for the specified number of
	// |milliseconds|.  To wait indefinetly, pass kForever.
	bool Wait(int milliseconds = kForever);

#if defined(WEBRTC_WIN)
	operator HANDLE()const
	{
		return event_handle_;
	}
#endif

private:
#if defined(WEBRTC_WIN)
	HANDLE event_handle_;
#elif defined(WEBRTC_POSIX)
	pthread_mutex_t event_mutex_;
	pthread_cond_t event_cond_;
	const bool is_manual_reset_;
	bool event_status_;
#endif
};
}
}
