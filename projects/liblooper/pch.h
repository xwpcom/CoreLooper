#pragma once
#if defined __cplusplus

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#if defined _MSC_VER
#define HAS_EPOLL //use wepoll
#endif

#ifndef bzero
#define bzero(ptr,size)  memset((ptr),0,(size));
#endif //bzero

#if defined(_WIN32)
#define FD_SETSIZE 1024 //修改默认64为1024路
#include <winsock2.h>
#include <windows.h>
#else
#include <cerrno>
#endif // defined(_WIN32)

#include <memory>
#include <functional>
#include <thread>
#include <string>
//#include <string_view>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <cassert>

#if defined _MSC_VER
#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  
#endif

#include "logger.h"

namespace Core {
using namespace std;
}

#endif
