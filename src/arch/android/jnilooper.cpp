#include "stdafx.h"
#include "arch/android/jnilooper.h"
#include "arch/android/jnihelper.h"

namespace Bear {
namespace Core {
JniLooper::JniLooper()
{
	SetObjectName("JniLooper");
	mThreadName = GetObjectName();
}

//说明:只应该在JniLooper此处调用一次 jni attach current thread,app其他地方都不应该再调用
void JniLooper::OnCreate()
{
	__super::OnCreate();

	JavaVM* jvm = AfxGetJavaVM();
	if (jvm)
	{
		PJniEnv env = nullptr;
		jvm->AttachCurrentThread(&env, nullptr);
	}
	else
	{
		DW("jvm is null");
		ASSERT(FALSE);
	}

	DV("JniLooper::OnCreate");
}

void JniLooper::OnDestroy()
{
	DW("%s", __func__);
	__super::OnDestroy();

	JavaVM* jvm = AfxGetJavaVM();
	if (jvm)
	{
		jvm->DetachCurrentThread();
	}
	else
	{
		DW("jvm is null");
		ASSERT(FALSE);
	}

}

void JniLooper::OnBMQuit()
{
	DW("%s", __func__);
	__super::OnBMQuit();
}

}
}
