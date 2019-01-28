#include "stdafx.h"
#include "arch/android/jnihelper.h"
#include "core/string//utf8tool.h"
using namespace Bear::Core;

tagJniInfo::tagJniInfo()
{
}

JNIEnv* tagJniInfo::GetJniEnv()
{
	JavaVM* jvm = AfxGetJavaVM();
	PJniEnv pEnv = NULL;

	if (jvm)
	{
		//https://docs.oracle.com/javase/1.5.0/docs/guide/jni/spec/invocation.html#attach_current_thread
		jvm->GetEnv((LPVOID*)&pEnv, JNI_VERSION_1_6);
		auto env = (JNIEnv *)pEnv;
		ASSERT(env);
		return env;
	}

	DW("fail %s", __func__);
	ASSERT(FALSE);
	return nullptr;
}

tagJniInfo::~tagJniInfo()
{
	//DV("%s#1", __func__);

	JavaVM* jvm = AfxGetJavaVM();
	PJniEnv pEnv = NULL;

	if (jvm)
	{
		//https://docs.oracle.com/javase/1.5.0/docs/guide/jni/spec/invocation.html#attach_current_thread
		jvm->GetEnv((LPVOID*)&pEnv, JNI_VERSION_1_6);
		auto env = (JNIEnv *)pEnv;
		if (env)
		{
			env->DeleteGlobalRef(mJavaObj);
			env->DeleteGlobalRef(mJavaClass);
		}
		else
		{
			DV("jni env is null,desc=[%s]", mDesc.c_str());//影响不大,如果能在JniLooper中析构tagJniInfo,则不会有此问题
		}
	}

	mJavaObj = NULL;
	mJavaClass = NULL;
	//DV("%s#2", __func__);
}

string JniHelper::GetJString(JNIEnv*env, jstring jstr)
{
	//DT("%s#1",__func__);
	char out[1024 * 4];

	int cbOut = sizeof(out);
	out[0] = 0;
	char *pBuf = out;
	bool useHeapBuf = false;

	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	//DT("alen=%d,cbOut=%d",alen,cbOut);
	if (alen > 0)
	{
		if (alen < cbOut - 1)
		{
			memcpy(out, ba, alen);
			out[alen] = 0;
		}
		else
		{
			pBuf = new char[alen + 1];
			memcpy(pBuf, ba, alen);
			pBuf[alen] = 0;
			useHeapBuf = true;
		}
	}
	env->ReleaseByteArrayElements(barr, ba, 0);

	env->DeleteLocalRef(clsstring);
	env->DeleteLocalRef(strencode);
	env->DeleteLocalRef(barr);
	//env->DeleteLocalRef(alen);

	string sz(pBuf);
	if (useHeapBuf)
	{
		delete[]pBuf;
		pBuf = nullptr;
	}
	//DT("%s#2",__func__);
	return sz;
}

jstring JniHelper::ToJString(JNIEnv* env, const char* pat)
{
	if (!pat || pat[0] == 0)
	{
		return 0;
	}

	jclass strClass = env->FindClass("java/lang/String");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray((jsize)strlen(pat));
	env->SetByteArrayRegion(bytes, 0, (jsize)strlen(pat), (jbyte*)pat);
	jstring encoding = env->NewStringUTF("utf-8");
	jstring js = (jstring)env->NewObject(strClass, ctorID, bytes, encoding);

	env->DeleteLocalRef(strClass);
	//env->DeleteLocalRef(ctorID);
	env->DeleteLocalRef(bytes);
	env->DeleteLocalRef(encoding);

	return js;
}

#ifndef _MSC_VER
#include <dlfcn.h> 
#endif

#ifndef LPCSTR 
typedef const char*     LPCSTR;
#endif 

#ifndef LPSTR 
typedef       char*     LPSTR;
#endif 

#ifndef s32 
typedef unsigned long   s32;
#endif 

typedef void(*pvUcnvFunc)
(LPCSTR lpcstrDstEcd, LPCSTR lpcstrSrcEcd,
	LPSTR lpstrOut, s32 nOutLen,
	LPCSTR lpstrIn, s32 nInLen, s32 *pnErrCode);

/* ucnv_convert method pointer */
static pvUcnvFunc g_pvUcnvConvert = NULL;

/* pointer libicuuc.so dl lib */
static void*      g_pvUcnvDll = NULL;

/*
see the source code define
int32_t  ucnv_convert(  const char *toEncoder,
const char *fromEncoder,
char *target,
int32_t targetSize,
const char *source,
int32_t sourceSize,
UErrorCode * err)
*/

//XiongWanPing 2017.02.07
//http://blog.csdn.net/xuhuan_wh/article/details/52170503
int UCNV_Utf8ToGB2312(LPSTR lpstrOut, s32 nOutLen, LPCSTR lpstrIn)
{
	s32 error = 0;
#ifndef _MSC_VER
	const char *soPath = "/system/lib/libicuuc.so";
	if (NULL == g_pvUcnvDll)
	{
		g_pvUcnvDll = dlopen(soPath, RTLD_LAZY);
	}

	if (NULL == g_pvUcnvDll)
	{
		DW("fail load %s", soPath);
		return -1;
	}

	if (NULL == g_pvUcnvConvert)
	{
		// here is Android 2.2 version, Android 2.1 version change to ucnv_convert_3_8
		//libicuuc的接口目前并没有公开，各android带的libicuuc.so其接口名称可能不同
		//可在.so中搜索ucnv_convert_字样找到api
		//经测试,mtk6572上面的是ucnv_convert_48
		for (int i = 40; i < 100; i++)
		{
			string name;
			name=StringTool::Format("ucnv_convert_%d", i);
			g_pvUcnvConvert = (pvUcnvFunc)dlsym(g_pvUcnvDll, name.c_str());
			if (g_pvUcnvConvert)
			{
				break;
			}
		}
	}

	if (NULL == g_pvUcnvConvert)
	{
		DW("g_pvUcnvConvert is null");
		return -1;
	}

	g_pvUcnvConvert("gb2312", "utf8", lpstrOut, nOutLen, lpstrIn, strlen(lpstrIn), &error);
#endif
	return error;
}

string JniHelper::Utf8ToGB2312(string utf8)
{
#ifdef _MSC_VER
	return Utf8Tool::UTF_8ToGB2312(utf8);
#endif
	//DW("%s,text=%s", __func__, utf8.c_str());

	string ret;

	try
	{
		int cbOut = (int)(utf8.length() * 2 + 100);
		char *out = new char[cbOut];
		memset(out, 0, cbOut);

		UCNV_Utf8ToGB2312(out, cbOut, utf8.c_str());
		//*
		string hex;
		for (int i = 0; i < cbOut; i++)
		{
			BYTE ch = out[i];
			if (ch == 0)
			{
				break;
			}
			StringTool::AppendFormat(hex, "%02x", ch);
		}
		//DW("GB2312ToUtf8 text=%s,hex=%s", utf8.c_str(),hex.c_str());
		//*/

		ret = out;
		delete[]out;
		out = nullptr;
	}
	catch (...)
	{
		DW("###catch");
	}

	return ret;
}

//SystemClock.elapsedRealtime()
ULONGLONG JniHelper::SystemClock_elapsedRealtime()
{
	int ret = -1;
	JavaVM* jvm = AfxGetJavaVM();
	//DV("jvm=%p", jvm);
	bool needDeatch = false;
	if (jvm)
	{
		PJniEnv pEnv = NULL;
		jvm->GetEnv((LPVOID*)&pEnv, JNI_VERSION_1_6);
		if(!pEnv)
		{
			PJniEnv env = nullptr;
			ret=jvm->AttachCurrentThread(&env, nullptr);
			//DV("AttachCurrentThread ret=%d",ret);
			needDeatch = true;
		}
	}

	auto env = tagJniInfo::GetJniEnv();

	jclass cls = env->FindClass("android/os/SystemClock");
	//注意获取static方法时要用GetStaticMethodID,非static方法要用GetMethodID
	auto mid = env->GetStaticMethodID(cls, "elapsedRealtime", "()J");
	//DV("cls=%p", cls);
	//DV("mid=%p", mid);
	auto tick = env->CallStaticLongMethod(cls, mid);
	env->DeleteLocalRef(cls);

	if (needDeatch)
	{
		jvm->DetachCurrentThread();
	}

	return tick;
}
