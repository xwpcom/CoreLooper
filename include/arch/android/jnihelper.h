#pragma once

#include <jni.h>
#ifdef _MSC_VER
typedef LPVOID PJniEnv;
#else
typedef JNIEnv *PJniEnv;
#endif

CORE_EXPORT JavaVM* AfxGetJavaVM();
void CORE_EXPORT AfxSetJavaVM(JavaVM* vm);

namespace Bear {
namespace Core {

struct CORE_EXPORT tagJniInfo
{
	tagJniInfo();
	virtual ~tagJniInfo();
	static JNIEnv* GetJniEnv();
	jclass  mJavaClass = nullptr;
	jobject	mJavaObj = nullptr;
	string	mDesc;//for debug use
};


#define _CONFIG_SP	//smart pointer

#ifdef _CONFIG_SP
//XiongWanPing 2016.07.08
//当采用shared_ptr时，采用struct tagXXX来封装智能指针对象,java mThisPtrID实际指向tagXXX
#define IMPL_JAVA_OBJ_2_CPP_OBJ(cppClassName)							\
struct tagSP_ ## cppClassName { shared_ptr<cppClassName> mSP;};			\
static  jfieldID  mThisPtrID = NULL;									\
static shared_ptr<cppClassName> JObj2CppObj(JNIEnv *env,jobject jObj)	\
{															\
	if(mThisPtrID==NULL)									\
	{														\
		DW("invalid mThisPtrID");							\
		return nullptr;										\
	}														\
															\
	auto ptr=env->GetLongField(jObj, mThisPtrID);			\
	if(ptr>=0 && ptr<=0xffff)								\
	{														\
		DW("invalid ptr");									\
		return nullptr;										\
	}														\
															\
	tagSP_ ## cppClassName *obj=(tagSP_ ## cppClassName *)ptr;\
	return obj->mSP;										\
}

#define J2CPP JObj2CppObj

#define JAVA_JNI_OBJ_INIT(env,jObj,cppClassName)				\
	jclass  pClass = env->GetObjectClass(jObj);					\
	if (pClass == NULL)											\
	{															\
		DW("[Discovery] failed GetObjectClass");				\
		return -1;												\
	}															\
																\
	if(mThisPtrID==NULL)										\
	{															\
		mThisPtrID = env->GetFieldID(pClass, "mThisPtr", "J");	\
		if (mThisPtrID == NULL)									\
		{														\
			DW("[Discovery]  failed GetFieldID");				\
			return -1;											\
		}														\
	}															\
																\
	auto ptr=env->GetLongField(jObj, mThisPtrID);				\
	if(ptr!=0)													\
	{															\
		DW("duplicate call initialize!");						\
		return -1;												\
	}															\
																\
	tagSP_ ## cppClassName *obj=new tagSP_ ## cppClassName;		\
	obj->mSP=make_shared<cppClassName>();						\
    env->SetLongField(jObj, mThisPtrID, (jlong)obj);

#define JAVA_JNI_OBJ_RELEASE(env,jObj,cppClassName)				\
	do{															\
		auto ptr=env->GetLongField(jObj, mThisPtrID);	\
		if(ptr>0xffff)											\
		{														\
			env->SetLongField(jObj, mThisPtrID, (jlong)0);			\
																\
			tagSP_ ## cppClassName *obj=(tagSP_ ## cppClassName*)ptr;	\
			delete obj;											\
		}														\
		else													\
		{														\
			DW("invalid release obj,ptr=0x%x",ptr);				\
		}														\
	}while(0)
#endif

//XiongWanPing 2013.05.13
class CORE_EXPORT JniHelper
{
public:
	static string GetJString(JNIEnv*env, jstring jstr);
	static jstring ToJString(JNIEnv* env, const char* pat);

	static string Utf8ToGB2312(string utf8);
	static ULONGLONG SystemClock_elapsedRealtime();
};

}
}
