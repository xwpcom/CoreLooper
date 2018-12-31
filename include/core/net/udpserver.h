#pragma once

#ifdef _MSC_VER
#include "arch/windows/udpserver_windows.h"
#define BASE_UDP_SERVER UdpServer_Windows
#else
#include "arch/linux/udpserver_linux.h"
#define BASE_UDP_SERVER UdpServer_Linux
#endif

namespace Bear {
namespace Core
{
namespace Net {
//XiongWanPing 2018.02.18
//用户自定义Udpserver应该从UdpServer或其子类继承
class CORE_EXPORT UdpServer :public BASE_UDP_SERVER
{

};
}
}
}