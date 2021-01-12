#include "stdafx.h"
#include "iocontext.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif
 
namespace Bear {
namespace Core
{
static const char* TAG = "IoContext";
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
}

IoContext::~IoContext()
{
}

int IoContext::PostSend()
{
#ifdef _MSC_VER
	ASSERT(!mBusying);
	ASSERT(mType == IoContextType_Send);
	if (!mBaseClient)
	{
		LogW(TAG,"invalid mBaseClient");
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
		
		mBusying = true;
		return 0;
	}
#endif

	mByteBuffer.clear();

	auto error = WSAGetLastError();
	LogW(TAG,"fail to send,,error=%d(%s)", error, SockTool::GetErrorDesc(error));
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
		if (ret == 0 && dwIoSize > 0)
		{
			/* 经测试，即使运行到此，仍然会收到此帧数据的IoContextType_Recv事件，所以此处不需要做处理 */
		}
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
}
}