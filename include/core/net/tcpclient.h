#pragma once
#ifdef _MSC_VER
#include "arch/windows/tcpclient_windows.h"
#define BASE_TCP_CLIENT TcpClient_Windows
#else
#include "arch/linux/tcpclient_linux.h"
#define BASE_TCP_CLIENT TcpClient_Linux
#endif

namespace Bear {
namespace Core
{
namespace Net {

class CORE_EXPORT TcpClient :public BASE_TCP_CLIENT
{

};
}
}
}