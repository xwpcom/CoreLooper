#include "stdafx.h"
#include "criticalsection.h"
#ifndef _MSC_VER
#include <pthread.h>
#endif

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

namespace Bear {
namespace Core
{

CriticalSection::CriticalSection()
{
	TRACK_OWNER(thread_ = 0);

#ifdef _MSC_VER
	::InitializeCriticalSection(&m_sect);
#else
	int ret = -1;
	UNUSED(ret);

	pthread_mutexattr_t attr;
	memset(&attr, 0, sizeof(attr));
	memset(&m_mutex, 0, sizeof(m_mutex));

	ret = pthread_mutexattr_init(&attr);
	ASSERT(ret == 0);

	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	ASSERT(ret == 0);

	int type = -1;
	ret = pthread_mutexattr_gettype(&attr, &type);
	ASSERT(ret == 0 && type == PTHREAD_MUTEX_RECURSIVE);

	ret = pthread_mutex_init(&m_mutex, &attr);
	ASSERT(ret == 0);

	ret = pthread_mutexattr_destroy(&attr);
	ASSERT(ret == 0);
#endif
}

CriticalSection::~CriticalSection()
{
#ifdef _MSC_VER
	::DeleteCriticalSection(&m_sect);
#else
	int ret = pthread_mutex_destroy(&m_mutex);
	if (ret)
	{
		DW("fail pthread_mutex_destroy ret=%d,error=%d(%s)", ret, errno, strerror(errno));
	}
	//ASSERT(ret==0);
#endif
}

void CriticalSection::Lock()
{
#ifdef _MSC_VER
	::EnterCriticalSection(&m_sect);
	TRACK_OWNER(thread_ = GetCurrentThreadId());
#else
	int ret = pthread_mutex_lock(&m_mutex);
	if (ret)
	{
		DW("error=%d(%s)", errno, strerror(errno));
		ASSERT(FALSE);
	}
	TRACK_OWNER(thread_ = pthread_self());
#endif
}

void CriticalSection::UnLock()
{
	TRACK_OWNER(thread_ = 0);

#ifdef _MSC_VER
	::LeaveCriticalSection(&m_sect);
#else
	int ret = pthread_mutex_unlock(&m_mutex);
	ASSERT(ret == 0);
	UNUSED(ret);
#endif
}
}
}
