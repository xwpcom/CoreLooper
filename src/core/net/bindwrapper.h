#pragma once

namespace Bear {
namespace Core
{
namespace Net {

#ifndef _MSC_VER
//typedef unsigned int size_t;
#endif

class BindWrapper
{
public:
	static int Bind(int sockfd, sockaddr* addr, size_t addrlen);
};

}
}
}
