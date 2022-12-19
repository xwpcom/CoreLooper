#pragma once

#include "core2.inl"

namespace Bear {
namespace Core2 {

class CORE_EXPORT Object
{
public:
	virtual ~Object() {}
};

typedef function<void()> TaskEntry;

//约定:
//采用LOOPER_SAFE修饰的接口可以安全的跨looper调用
//没有采用LOOPER_SAFE修饰的接口，不保证跨looper安全调用，应该只在handler所在looper里调用
#define LOOPER_SAFE	//表示handler创建后接口是跨looper安全的

class CORE_EXPORT Handler:public Object, public enable_shared_from_this<Handler>
{
public:
	virtual LOOPER_SAFE void post(TaskEntry t, UINT ms = 0) {}

};

}
}