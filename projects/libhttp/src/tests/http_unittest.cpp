#include "stdafx.h"
#include "CppUnitTest.h"
#include "libhttp.inl"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net::Http;

namespace HttpUnitTest
{

class Ajax_Info :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_Info)
protected:
	string Process(const NameValue& params)
	{

		XmlString x;
		x.AddItem("Name", "Bear");
		x.AddItem("Age", "37");

		auto ack = StringTool::Format(
			"<Result><Error>0</Error>%s</Result>"
			,x.GetString().c_str()
		);
		return std::move(ack);
	}
};

IMPLEMENT_AJAX_CLASS(Ajax_Info, "Info", "")


TEST_CLASS(Http)
{
public:
	TEST_METHOD(TestHttpServer)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto svr(make_shared<HttpServer>());
				string name = "WebServerX";
				AddChild(svr, name.c_str());
				{
					string webRootFolder = "g:/test/web/";
					//webRootFolder=StringTool::Format("%s/html", ShellTool::GetAppPath().c_str());
					DV("webRootFolder=%s", webRootFolder.c_str());

					auto config = make_shared<tagWebServerConfig>();
					auto ajaxHandler = make_shared<AjaxCommandHandler>();
					config->mWebRootFolder = webRootFolder.c_str();
					config->mAjaxCommandHandler = ajaxHandler;
					/*
					{
						shared_ptr<VirtualFolder> vm = make_shared<VirtualFolder>();
						vm->AddMount("sd", GetMediaRootPath());
						config->mVirtualFolder = vm;

						config->mMediaRootPath = GetMediaRootPath().c_str();
					}
					*/
					svr->SetConfig(config);
					svr->AddChild(ajaxHandler);
				}

				int ret = svr->StartServer(8080);

				class DelayExit :public Runnable
				{
					void Run()
					{
						Looper::CurrentLooper()->PostQuitMessage();
					}
				};

				postDelayedRunnable(make_shared<DelayExit>(), 10 * 1000);
			}
		};

		make_shared<MainLooper>()->StartRun();

	}
};

}
