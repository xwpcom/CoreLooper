#include "stdafx.h"
#include "UdpIoContext.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif
namespace Bear {
namespace Core
{
namespace Net {


UdpIoContext::UdpIoContext()
{
	bzero(&mSockAddr, sizeof(mSockAddr));
}

UdpIoContext::~UdpIoContext()
{
}

int UdpIoContext::PostRecv(int maxRecvBytes)
{
#ifdef _MSC_VER
	ASSERT(!mBusying);
	ASSERT(!mBaseClient || !mBaseServer);

	DWORD dwIoSize = 0;
	DWORD dwFlags = 0;

	memset(&mOV, 0, sizeof(mOV));
	WSABUF wb;
	mByteBuffer.clear();
	wb.buf = (char*)mByteBuffer.GetNewDataPointer();
	if (maxRecvBytes == 0)
	{
		maxRecvBytes = mByteBuffer.GetBufferSize() - 1;
	}
	wb.len = MIN(maxRecvBytes, mByteBuffer.GetBufferSize() - 1);

	ASSERT(wb.len > 0);
	mAddrLen = sizeof(mSockAddr);
	int ret = WSARecvFrom(mSock, &wb, 1, &dwIoSize, &dwFlags, &mSockAddr, &mAddrLen, &mOV, NULL);
	if (ret == 0 || (ret == -1 && WSA_IO_PENDING == WSAGetLastError()))
	{
		mBusying = true;
		return 0;
	}
#endif

	return -1;
}
}
}
}