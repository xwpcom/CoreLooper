#include "stdafx.h"
#include "linuxsignal.h"

namespace Bear {
namespace Core
{

#ifndef _CONFIG_ANDROID
#ifndef _MSC_VER
void handle_sigterm(int sig)
{
	DW("handle_sigterm,sig=%s", LinuxSignal::GetSigName(sig));
	if (sig != SIGPIPE)
	{
		exit(1);
	}
}
#endif

LinuxSignal::LinuxSignal()
{
}

LinuxSignal::~LinuxSignal()
{
}

int LinuxSignal::InitEx()
{
#ifndef _MSC_VER
	signal(SIGPIPE, SIG_IGN);//#define SIGPIPE		13	// Broken pipe (POSIX)
#endif

	return 0;
}

//把所有信号都重置为default状态

int LinuxSignal::ResetAll()
{
#ifndef _MSC_VER
	for (int sig = 1; sig < 32; sig++)
	{
		if (sig == SIGKILL || sig == SIGSTOP)
		{
			continue;
		}
#ifdef _RUN_AT_PC_LINUX
		if (sig == 20)//ubuntu下面按ctrl+z
		{
			continue;
		}
#endif		
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_DFL;
		//sigfillset(&act.sa_mask);//屏蔽所有其他signal

		int ret = sigaction(sig, &act, NULL);
		if (ret == 0)
		{
			DW("sigaction sig=%d", sig);
		}
		else
		{
			DW("fail sigaction,sig=%s,errno=%d(%s)",
				GetSigName(sig),
				errno,
				strerror(errno)
			);
		}
	}
#endif

	return 0;
}

BOOL LinuxSignal::IsSpecialSignal(int sig)
{
#ifndef _MSC_VER
	static int arr[] =
	{
		SIGKILL,
		SIGSTOP,
		SIGBUS,
		//SIGSEGV,
		//SIGFPE,
		//SIGILL,
#ifdef _RUN_AT_PC_LINUX
		SIGTERM,
		SIGSTOP,
		SIGWINCH,
#endif
	};

	for (int i = 0; i < COUNT_OF(arr); i++)
	{
		if (arr[i] == sig)
		{
			return TRUE;
		}
	}
#endif

	return FALSE;
}

const char *LinuxSignal::GetSigName(int sig)
{
#ifdef _MSC_VER
	return "";
#else
	return strsignal(sig);
#endif

	/*
	static const char *arr[]=
	{
		"0",
		"SIGHUP",
		"SIGINT",
		"SIGQUIT",
		"SIGILL",
		"SIGTRAP",
		"SIGABRT",
		"SIGIOT",
		"SIGBUS",
		"SIGFPE",
		"SIGKILL",
		"SIGUSR1",
		"SIGSEGV",
		"SIGUSR2",
		"SIGPIPE",
		"SIGALRM",
		"SIGTERM",
		"SIGSTKFLT",
		"SIGCHLD",
		"SIGCONT",
		"SIGSTOP",
		"SIGTSTP",
		"SIGTTIN",
		"SIGTTOU",
		"SIGURG",
		"SIGXCPU",
		"SIGXFSZ",
		"SIGVTALRM",
		"SIGPROF",
		"SIGWINCH",
		"SIGPOLL",
		"SIGPWR",
		"SIGSYS",
	};

	if(sig>=0 && sig<COUNT_OF(arr))
	{
		return arr[sig];
	}

	DW("unknown sig=%d",sig);
	return "unknown sig";
	//*/
}

void LinuxSignal::SigAction(int sig, siginfo_t* sigInfo, void * )
{
	const int errnoSave = errno;

#ifndef _MSC_VER
	DW("\n\n\n\n\n");
	DW("###sig_handler,sig=%d(%s),sigInfo=%p\n",
		sig,
		GetSigName(sig),
		sigInfo
	);
	DW("si_signo=%d,si_code=%d,"
		//"si_value=0x%x,"
		"si_errno=%d,"
		"si_pid=0x%x,si_uid=0x%x,si_addr=0x%p,si_status=0x%x,si_band=0x%x"
		"%s\n",
		sigInfo->si_signo,
		sigInfo->si_code,
		//sigInfo->si_value,
		(unsigned int)sigInfo->si_errno,
		sigInfo->si_pid,
		sigInfo->si_uid,
		(long*)sigInfo->si_addr,
		sigInfo->si_status,
		(unsigned int)sigInfo->si_band,
		""
	);

	switch (sig)
	{
	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	case SIGBUS:
	case SIGTRAP:
	{
		DW("crash addr=%p\n", sigInfo->si_addr);
		break;
	}
	default:
	{
		break;
	}
	}

	//man sigaction上面说忽略SIGFPE,SIGILL和SIGSEGV信号的话，程序行为将不确定
	//所以打印出crash addr后就自动退出程序
	if (sig == SIGFPE || sig == SIGILL || sig == SIGSEGV
#ifdef _SONIX986
		|| sig == SIGSTOP
#endif
		)
	{
		exit(0);
	}

	DW("\n\n\n\n\n");
	errno = errnoSave;
#endif
}

int LinuxSignal::Init()
{
#ifndef _MSC_VER

	for (int i = 1; i < 32; i++)
	{
		int sig = i;
		if (!IsSpecialSignal(sig))
		{
#ifdef _RUN_AT_PC_LINUX
			if (sig == 20)
			{
				continue;
			}
#endif
			struct sigaction act;
			memset(&act, 0, sizeof(act));

			act.sa_sigaction = SigAction;
			act.sa_flags = SA_NOCLDWAIT | SA_RESTART | SA_SIGINFO;

			int ret = sigaction(sig, &act, NULL);
			if (ret)
			{
				DW("fail sigaction,sig=%s,errno=%d(%s)",
					GetSigName(sig),
					errno,
					strerror(errno)
				);
			}
		}
	}

	{
		//屏蔽掉不需要的linux特性.
		signal(SIGPIPE, SIG_IGN);//#define SIGPIPE		13	/* Broken pipe (POSIX).  */
		signal(SIGCHLD, SIG_IGN);//#define SIGCHLD		18	/* Child status has changed (POSIX).  */
#ifndef _RUN_AT_PC_LINUX
		signal(SIGHUP, handle_sigterm);//#define SIGHUP		 1	/* Hangup (POSIX).  */
		//signal( SIGUSR1, handle_sigterm );//#define SIGUSR1		16	/* User-defined signal 1 (POSIX).  */

		signal(SIGSTOP, SIG_IGN);//handle_sigterm);//CTRL+Z,没效果//#define SIGSTOP		23	/* Stop, unblockable (POSIX).  */
		signal(SIGTERM, handle_sigterm);	//#define SIGTERM		15	/* Termination (ANSI).  */
		signal(SIGINT, SIG_IGN);//handle_sigterm );//CTRL+C//#define SIGINT		 2	/* Interrupt (ANSI).  */
#endif
	}

#endif

	return 0;
}

#ifdef _DEBUG
int LinuxSignal::Test()
{
	Init();

	int recvSignalTimes = 0;
	while (recvSignalTimes < 10)
	{
		//	pause();

		recvSignalTimes++;
		if (recvSignalTimes == 3)
		{
			DT("test divide 0#begin");
			int value = 0;
			int x = 1 / value;
			int xx = x;

			DT("test divide 0#end");
		}
		//DT("recvSignalTimes=%d",recvSignalTimes);
	}

	return -1;
}
#endif
#endif
}
}
