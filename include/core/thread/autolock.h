#pragma once
namespace Bear {
namespace Core
{
class CriticalSection;
class CORE_EXPORT AutoLock
{
public:
	AutoLock(CriticalSection *pcs);
	virtual ~AutoLock();
protected:
	CriticalSection * m_pcs;
};
}
}