#pragma once

#ifdef _MSC_VER
#include "arch/windows/tcpserver_windows.h"
#define BASE_TCP_SERVER TcpServer_Windows
#else
#include "arch/linux/tcpserver_linux.h"
#define BASE_TCP_SERVER TcpServer_Linux
#endif

namespace Bear {
namespace Core
{
namespace Net {
//XiongWanPing 2016.03.28
//用户自定义tcpserver应该从TcpServer或其子类继承
class CORE_EXPORT TcpServer :public BASE_TCP_SERVER
{

};
}
}
}