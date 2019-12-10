#pragma once

namespace Bear {
using namespace Bear::Core;
namespace Android {

/*
XiongWanPing 2019.08.30
JniMainLooper用法:
在java中通过jni接口
.通过JniMainLooper::CheckCreateJniMainLooper()来创建
.JniMainLooper作为c++ main looper,一直运行，不提供退出机制
.java native类对象映射到c++ handler对象,此handler对象绑定在JniMainLooper中，可以create,destroy
*/
class CORE_EXPORT JniMainLooper :public MainLooper_
{
	SUPER(MainLooper_);
public:
	JniMainLooper();

	static void CheckCreateJniMainLooper();
	static shared_ptr<Handler> gInstance;
protected:
	void OnCreate();
};
}
}
