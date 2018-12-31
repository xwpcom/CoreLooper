#include "stdafx.h"
#include "thread/autolock.h"
#include "criticalsection.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif
namespace Bear {
namespace Core
{

AutoLock::AutoLock(CriticalSection *pcs)
{
	m_pcs = pcs;
	ASSERT(m_pcs);
	if (m_pcs)
	{
		m_pcs->Lock();
	}
}

AutoLock::~AutoLock()
{
	if (m_pcs)
	{
		m_pcs->UnLock();
		m_pcs = NULL;
	}
}
}
}
