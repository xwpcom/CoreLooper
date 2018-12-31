#include "stdafx.h"
#include "iocpobject.h"
#include "looper/looper.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

namespace Bear {
namespace Core
{
namespace Net {
IocpObject::IocpObject()
{
}

IocpObject::~IocpObject()
{

}

int IocpObject::DispatchIoContext(IoContext *context, DWORD bytes)
{
	return -1;
}

/*
BOOL IocpObject::PostCustomIocpMessage(HANDLE iocp, LPVOID obj, UINT msg, LPVOID info)
{
	ASSERT(msg <= 0xFFFF);
	BOOL ok = false;
#ifdef _MSC_VER
	//由于win32 PostQueuedCompletionStatus所限，这里只能采用DWORD传递指针
	//win32环境下可以这样做，如果是x64 app则不行
	ASSERT(sizeof(DWORD) == sizeof(long*));
	ok = PostQueuedCompletionStatus(iocp, (DWORD)info, msg, (LPOVERLAPPED)obj);
#else
	ASSERT(FALSE);
#endif
	return ok;
}
*/

void IocpObject::OnCustomIocpMessage(UINT msg, LPVOID info)
{
	//子类在处理时需要调用auto objThis = shared_from_this();确保不会被提前删除
}
}
}
}
