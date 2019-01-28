#include "stdafx.h"
#include "CppUnitTest.h"
#include "libhttp.inl"
#include "libhttp/httpget.h"

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
	TEST_METHOD(Parse)
	{
		string url = "/tree.xml?url=DemoHandler";
		string uri;
		NameValue params;
		HttpTool::ParseUrlParam(url, uri, params);
		Assert::AreEqual(params.GetString("url").c_str(),"DemoHandler");
		params.Dump();
	}

	TEST_METHOD(HttpServerDemo)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto svr(make_shared<HttpServer>());
				AddChild(svr);
				
				{
					string webRootFolder = "g:/test/web/";
					auto config = make_shared<tagWebServerConfig>();
					auto ajaxHandler = make_shared<AjaxCommandHandler>();
					config->mWebRootFolder = webRootFolder.c_str();
					config->mAjaxCommandHandler = ajaxHandler;
					svr->SetConfig(config);
					svr->AddChild(ajaxHandler);
				}

				int ret = svr->StartServer(80);

				{
					class DelayExit :public Runnable
					{
						void Run()
						{
							Looper::CurrentLooper()->PostQuitMessage();
						}
					};

					postDelayedRunnable(make_shared<DelayExit>(), 30 * 1000);
				}
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(Tree)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto svr(make_shared<HttpServer>());
				AddChild(svr);

				{
					string webRootFolder = "g:/test/web/";
					auto config = make_shared<tagWebServerConfig>();
					auto ajaxHandler = make_shared<AjaxCommandHandler>();
					config->mWebRootFolder = webRootFolder.c_str();
					config->mAjaxCommandHandler = ajaxHandler;
					svr->SetConfig(config);
					svr->AddChild(ajaxHandler);
				}

				int ret = svr->StartServer(80);

				{
					class DelayExit :public Runnable
					{
						void Run()
						{
							Looper::CurrentLooper()->PostQuitMessage();
						}
					};

					postDelayedRunnable(make_shared<DelayExit>(), 30 * 1000);
				}

				{
					class DemoHandler :public HandlerEx
					{
						SUPER(HandlerEx);

						void OnCreate()
						{
							__super::OnCreate();

							SetObjectName("DemoHandler");
							SetTimer(mTimerTest, 1000);

							BindProcData(mData, "mData");
							BindProcData(mString, "mString");
						}

						void OnTimer(long id)
						{
							if (id == mTimerTest)
							{
								mData++;

								auto now = ShellTool::GetCurrentTimeMs();
								mString = StringTool::Format("%02d:%02d:%02d",now.hour,now.minute,now.second);
								return;
							}

							__super::OnTimer(id);
						}

					protected:
						int mData=0;
						string mString;
						long mTimerTest=0;
					};

					AddChild(make_shared<DemoHandler>());
				}
			}
		};

		make_shared<MainLooper>()->StartRun();
	}
	
	TEST_METHOD(HttpDownload)
	{
		//测试http下载速度,可用来测试设备和电脑之间的网络连接速度，稳定性等指标
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj(make_shared<HttpGet>());
				AddChild(obj);

				obj->SignalHttpGetAck.connect(this,&MainLooper::OnHttpAck);

				string url = "http://192.168.1.9:8080/cool2.1.rar";
				url = "http://download.agora.io/sdk/release/Agora_Native_SDK_for_Windows_v2_2_1_FULL.zip";
				//url = "http://www.nsfocus.com.cn/";
				string saveAsFilePath = "d:/x.rar";
				obj->Execute(url, saveAsFilePath);
			}

			void OnHttpAck(HttpGet* obj, string&, int error, ByteBuffer&)
			{
				if (error == 0)
				{
					DW("download success,speed=%.1f KB/S", obj->GetSpeed());
				}
				else
				{
					DW("download fail,error=%d", error);
				}

				PostQuitMessage();
			}
		};

		make_shared<MainLooper>()->StartRun();
	}
};

}
