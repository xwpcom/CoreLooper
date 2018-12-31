#pragma once

#ifdef POSIX
#include <pthread.h>
#endif

#include "thread/autolock.h"
namespace Bear {
namespace Core
{
#if defined _DEBUG
#define CS_TRACK_OWNER 1
#endif  // _DEBUG

#if CS_TRACK_OWNER
#define TRACK_OWNER(x) x
#else  // !CS_TRACK_OWNER
#define TRACK_OWNER(x)
#endif  // !CS_TRACK_OWNER

//XiongWanPing 2010.04.02
//CriticalSection can be reenterable in the same thread
class CORE_EXPORT CriticalSection
{
public:
	CriticalSection();
	virtual ~CriticalSection();

	void UnLock();
	void Unlock()
	{
		UnLock();
	}
	void Lock();

	void Enter() { Lock(); }
	void Leave() { UnLock(); }

#if CS_TRACK_OWNER
#ifdef _MSC_VER
	bool CurrentThreadIsOwner() const { return thread_ == GetCurrentThreadId(); }
#else
	bool CurrentThreadIsOwner() const { return pthread_equal(thread_, pthread_self()); }
#endif
#endif  // CS_TRACK_OWNER

protected:

#ifdef _MSC_VER
	CRITICAL_SECTION	m_sect;
	TRACK_OWNER(DWORD thread_);  // The section's owning thread id
#else
	pthread_mutex_t		m_mutex;
	TRACK_OWNER(pthread_t thread_);
#endif
};

}
}