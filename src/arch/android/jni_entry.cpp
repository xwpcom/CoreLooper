#include "stdafx.h"
#include <jni.h>

using namespace Bear::Core;

#ifdef _CONFIG_JNI
JavaVM* g_JavaVM = NULL;
JavaVM* AfxGetJavaVM()
{
	return g_JavaVM;
}

#ifndef AV_LOG_TRACE
#define AV_LOG_TRACE    56
extern "C" void av_log_set_level(int level);
#endif

extern "C" int av_jni_set_java_vm(void *vm, void *log_ctx);//ffmpeg h264_mediacodec需要调用
extern "C" void av_log_set_callback(void(*callback)(void*, int, const char*, va_list));

static void ffmpeg_log(void* contextAVClass, int level, const char* fmt, va_list varargs)
{
	va_list vl2;

	char buf[16 * 1024];
	va_copy(vl2, varargs);
	vsnprintf(buf, sizeof(buf) - 1, fmt, vl2);
	va_end(vl2);

	DV("###%s", buf);
}


#ifdef _MSC_VER
int av_jni_set_java_vm(void *vm, void *log_ctx)
{
	return 0;
}
#endif


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	DV("JNI_OnLoad#bear.2020.06.23,compile time=[%s %s],jvm=%p", __DATE__, __TIME__,vm);
	//av_jni_set_java_vm(vm, nullptr);
	//av_log_set_level(AV_LOG_TRACE);
	//av_log_set_callback(ffmpeg_log);

	g_JavaVM = vm;
	return JNI_VERSION_1_6;
}

#endif
