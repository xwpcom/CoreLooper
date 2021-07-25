#pragma once

#ifdef _MSC_VER
#define _CONFIG_OPENSSL //2019.12,用来支持https做微信小程序
#define ENABLE_OPENSSL //for zltoolkit
#endif


#ifndef _MSC_VER
#ifndef NDEBUG
	#ifndef _DEBUG
		#define _DEBUG
	#endif
#else
#undef _DEBUG
#endif
#endif

#if !defined _CONFIG_ANDROID && defined __clang__ && !defined __APPLE__
//android studio use clang compile now
#define _CONFIG_ANDROID
#endif

#if defined _MSC_VER || defined _CONFIG_PC_LINUX || defined _CONFIG_HI3516 || defined _CONFIG_INGENIC || defined _CONFIG_ANDROID
#define _CONFIG_FFMPEG
#endif

#if defined _MSC_VER || defined _CONFIG_ANDROID
#define _CONFIG_JNI
#endif

#if defined _CONFIG_INGENIC
	#define __STDC_CONSTANT_MACROS
#endif

#ifdef _CONFIG_BOOST_SP
//boost sp is implemented in header
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
//#include <boost/signals2.hpp>
#define CPP_SP boost
#else
#define CPP_SP std
#endif

#ifdef _MSC_VER
#else

#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifndef __APPLE__
#include <sys/types.h>
#include <linux/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif
#include <sys/types.h>
#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <netinet/tcp.h>   //   for   TCP_NODELAY  
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#endif

#ifdef _SDK_DLL
#define CORE_EXPORT	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define CORE_EXPORT	__declspec(dllimport)
#else
#define CORE_EXPORT
#endif
#endif

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#ifndef _MSC_VER
#include "core/base/win32.h"
typedef unsigned long long ULONGLONG;
#define __FUNCSIG__ __func__
#define INVALID_HANDLE_VALUE -1
#define __cdecl

namespace Bear {
namespace Core
{

typedef struct _OVERLAPPED {
	ULONG_PTR Internal;
	ULONG_PTR InternalHigh;
	union {
		struct {
			DWORD Offset;
			DWORD OffsetHigh;
		} DUMMYSTRUCTNAME;
		PVOID Pointer;
	} DUMMYUNIONNAME;

	HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#define WSAOVERLAPPED           OVERLAPPED
typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

typedef struct _WSABUF {
	ULONG len;     /* the length of the buffer */
	CHAR  *buf; /* the pointer to the buffer */
} WSABUF, *LPWSABUF;
}
}
#endif

#ifdef _CONFIG_BOOST
#include <boost/timer/timer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#endif

#include <list>
#include <queue>
#include <string>
#include <memory>
#include <chrono>  
#include <thread> 
#include <mutex>

//disable copy
#define DISABLE_COPY_CLASS(cls)		\
private:							\
	cls(const cls&); 				\
	cls& operator=(const cls&);		

#include "core/base/shelltool.h"
#include "core/base/dt.h"
#include "core/thread/autolock.h"
#include "core/thread/criticalsection.h"
#include "core/base/bytebuffer.h"
//#include "core/stringex.h"
//#include "core/base/keyvalue.h"
#include "core/base/namevalue.h"
#include "core/base/stringtool.h"
#include "core/base/tickdumper.h"
//#include "core/listex.h"
#include "core/net/socktool.h"
#include "core/file/file.h"
#include "core/file/inifile.h"
#include "core/file/dumpfile.h"
#include "core/file/filefinder.h"
#include "core/base/bundle.h"
#include "core/base/bundleex.h"
#include "core/thread/event.h"
#include "core/base/log.h"
#include "core/base/fieldtracker.h"
//#include "core/string/Utf8Tool.h"
#include "core/string/stringcasemap.h"
#include "core/string/stringparam.h"
#include "core/string/textprotocol.h"
#include "core/string/textseparator.h"
#include "core/string/xmlstring.h"
#include "core/looper/handler.h"
#include "core/looper/looper.h"
#include "core/looper/blocklooper.h"
#include "core/looper/asynctasklooper.h"
#include "core/net/channel.h"
#include "json/ArduinoJson.h"
#include "json/jsonhelper.h"
#include "core/base/bytetool.h"
