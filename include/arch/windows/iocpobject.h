#pragma once
#include "core/looper/handler.h"
namespace Bear {
namespace Core
{
namespace Net {
class IoContext;
//XiongWanPing 2016.03.~
//IocpObject在有未决事务时都会维护一个ref count
class CORE_EXPORT IocpObject
{
public:
	IocpObject(){}
	virtual ~IocpObject(){}

	virtual int DispatchIoContext(IoContext *context, DWORD bytes)=0;
};
}
}
}