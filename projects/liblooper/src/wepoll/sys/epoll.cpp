#include "pch.h"
#include "epoll.h"
namespace Core{
std::map<int, HANDLE> s_wepollHandleMap;
int s_handleIndex = 0;
std::mutex s_handleMtx;

}
