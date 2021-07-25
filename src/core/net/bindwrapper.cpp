#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#endif

#include "bindwrapper.h"

namespace Bear {
namespace Core
{
namespace Net {

//avoid conflict with std::bind
int BindWrapper::Bind(int sockfd, sockaddr* addr, size_t addrlen)
{
	auto ret = bind(sockfd, (sockaddr*)addr, addrlen);
	return ret;
}

}
}
}
