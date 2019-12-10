#pragma once
#ifdef _MSC_VER
#include "arch/windows/tcpclient_windows.h"
#define TcpClient TcpClient_Windows
#else
#include "arch/linux/tcpclient_linux.h"
#define TcpClient TcpClient_Linux
#endif

namespace Bear {
namespace Core
{
namespace Net {

}
}
}