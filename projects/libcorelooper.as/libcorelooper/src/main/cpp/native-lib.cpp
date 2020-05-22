#include <jni.h>
#include <string>
#include "looper/libcorelooper.inl"
#include <core/net/tcpserver.h>
using namespace Bear::Core;
using namespace Bear::Core::Net;

class MainLooper:public MainLooper_
{
    SUPER(MainLooper_)
public:
	shared_ptr<Event> mSelfExitEvent;
protected:
    void OnCreate()
    {
        __super::OnCreate();

        SetTimer(mTimerTest,1000);

		{
			auto svr(make_shared<TcpServer>());
			{
				svr->SetObjectName("WebServer");
				DV("add web server");
				AddChild(svr);
			}

			int ret = svr->StartServer(8080);
		}

    }

    void OnTimer(long id)
    {
        if(id==mTimerTest)
        {
            static int idx=-1;
            ++idx;
            DV("%s,idx=%04d",__func__,idx);
            
            return;
        }

        __super::OnTimer(id);
    }

    long mTimerTest=0;
};

extern "C" JNIEXPORT jstring
JNICALL
Java_com_jjyip_libcorelooper_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

	if(!Looper::GetMainLooper())
    {
		shared_ptr<Event> event;
		{
			auto obj=make_shared<MainLooper>();
			obj->mSelfExitEvent = make_shared<Event>(true, false);
			obj->SetExitEvent(obj->mSelfExitEvent);
			obj->Start();
			event=obj->GetExitEvent();

			//obj->PostQuitMessage();
		}

		/*
		if (event)
		{
			DV("wait looper exit#begin");
			event->Wait();
			DV("wait looper exit#end");
		}

		auto obj = Looper::CurrentLooper();
    	if(obj)
		{
			DW("current looper is exists");
		} else{
			DV("current looper is null");
    	}
		//Looper::SetCurrentLooper(nullptr);
		 //*/
    }

    static int idx=0;
	++idx;

    std::string hello = StringTool::Format("corelooper,times=%d",idx);
    return env->NewStringUTF(hello.c_str());


}
