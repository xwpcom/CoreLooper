#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#if defined _MSC_VER
#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  
#endif

namespace Core {
using namespace std;
}
