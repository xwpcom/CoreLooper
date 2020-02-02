#pragma once

#ifdef _MSC_VER
typedef int siginfo_t;
#else
#include <signal.h>
#endif

namespace Bear {
namespace Core
{

//XiongWanPing 2012.07.06
//.采用sigaction()而不是signal()来屏蔽所有不需要的事件
//除了SIGBUS, SIGSEGV, SIGFPE这三个之外，其他的都要屏蔽

class CORE_EXPORT LinuxSignal
{
public:
	LinuxSignal();
	virtual ~LinuxSignal();

	int Init();
	int InitEx();

	static const char *GetSigName(int sig);

#ifdef _DEBUG
	int Test();
#endif

	static int ResetAll();

protected:
	static void SigAction(int sig, siginfo_t* sigInfo, void * unused);
	static BOOL IsSpecialSignal(int sig);
};
}
}