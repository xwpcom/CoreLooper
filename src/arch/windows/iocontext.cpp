#include "stdafx.h"
#include "iocontext.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif
 
namespace Bear {
namespace Core
{
namespace Net {
#ifdef _DEBUGXX
static long gIoContextRC = 0;
#endif

IoContext::IoContext()
{
	mBusying = false;
	mSock = INVALID_SOCKET;
	mType = IoContextType_None;
	memset(&mOV, 0, sizeof(mOV));

	mByteBuffer.SetBufferSize(1024 * 4, 1024 * 16);

#ifdef _DEBUGXX
	long rc = InterlockedIncrement(&gIoContextRC);
	DV("%s,gIoContextRC = %ld,this=0x%08x", __func__, gIoContextRC, this);
	if (rc == 4)
	{
		int x = 0;
	}
#endif
}

IoContext::~IoContext()
{
#ifdef _DEBUGXX
	long rc = InterlockedDecrement(&gIoContextRC);
	DV("%s,gIoContextRC = %ld,this=0x%08x", __func__, gIoContextRC, this);
#endif
}

int IoContext::PostSend()
{
#ifdef _MSC_VER
	ASSERT(!mBusying);
	ASSERT(mType == IoContextType_Send);
	if (!mBaseClient)
	{
		DV("invalid mBaseClient");
		mByteBuffer.clear();
		return -1;
	}

	DWORD dwIoSize = 0;
	DWORD dwFlags = 0;

	memset(&mOV, 0, sizeof(mOV));
	WSABUF wb;
	wb.buf = (char*)mByteBuffer.GetDataPointer();
	wb.len = mByteBuffer.GetActualDataLength();
	ASSERT(wb.len > 0);
	//DV("WSASend,bytes=%d",wb.len);
	int ret = -1;
	if (mSocketType)
	{
		ret = WSASend(mSock, &wb, 1, NULL, dwFlags, &mOV, NULL);
	}
	else
	{
		auto ok = WriteFile((HANDLE)mSock, wb.buf, wb.len, &dwIoSize, &mOV);
		if (ok)
		{
			ret = 0;
		}
	}
	if (ret == 0 || (ret == -1 && WSA_IO_PENDING == WSAGetLastError()))
	{
		//DV("WSASend,len=%04d", wb.len);
		mBusying = true;
		return 0;
	}
#endif

	DW("fail to send,maybe socket is closed");
	mByteBuffer.clear();
	return -1;
}

int IoContext::PostRecv(int maxRecvBytes)
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
		maxRecvBytes = mByteBuffer.GetBufferSize();
	}
	wb.len = MIN(maxRecvBytes, mByteBuffer.GetBufferSize());

	ASSERT(wb.len > 0);
	int ret = -1;
	if (mSocketType)
	{
		ret = WSARecv(mSock, &wb, 1, &dwIoSize, &dwFlags, &mOV, NULL);
	}
	else
	{
		auto ok = ReadFile((HANDLE)mSock, wb.buf, wb.len, &dwIoSize, &mOV);
		if (ok)
		{
			ret = 0;
		}
	}
	if (ret == 0 || (ret == -1 && WSA_IO_PENDING == WSAGetLastError()))
	{
		mBusying = true;
		return 0;
	}
#endif

	return -1;
}
}
}}