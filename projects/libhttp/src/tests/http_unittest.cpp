#include "stdafx.h"
#include "CppUnitTest.h"
#include "libhttp.inl"
#include "libhttp/httpget.h"
#include "parser_unittest.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Http;
using namespace mediakit;

namespace mediakit {

string FindField(const char* buf, const char* start, const char* end, int bufSize) {
	if (bufSize <= 0) {
		bufSize = strlen(buf);
	}
	const char* msg_start = buf, * msg_end = buf + bufSize;
	int len = 0;
	if (start != NULL) {
		len = strlen(start);
		msg_start = strstr(buf, start);
	}
	if (msg_start == NULL) {
		return "";
	}
	msg_start += len;
	if (end != NULL) {
		msg_end = strstr(msg_start, end);
		if (msg_end == NULL) {
			return "";
		}
	}
	return string(msg_start, msg_end);
}

}//namespace mediakit
vector<string> split(const string& s, const char* delim) {
	vector<string> ret;
	int last = 0;
	int index = s.find(delim, last);
	while (index != string::npos) {
		if (index - last > 0) {
			ret.push_back(s.substr(last, index - last));
		}
		last = index + strlen(delim);
		index = s.find(delim, last);
	}
	if (s.size() - last > 0) {
		ret.push_back(s.substr(last));
	}
	return ret;
}

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

static const char* TAG = "Http";

TEST_CLASS(Http)
{
public:
	TEST_METHOD(ParseHeader)
	{
		Parser obj;
		obj.Parse(
			"GET /index.html HTTP/1.1\r\n"
			"Name: bear\r\n"
			"name: xwp\r\n"
			"\r\n"
		);

		DV("Name=%s", obj["Name"].c_str());
		DV("name=%s", obj["name"].c_str());
		DV("player=%s", obj["player"].c_str());
		
		//map<string,string> //用map能实现大小写无关吗，有待测试
	}

	TEST_METHOD(ParseUrl)
	{
		string url = "/tree.xml?url=DemoHandler";
		string uri;
		NameValue params;
		HttpTool::ParseUrlParam(url, uri, params);
		Assert::AreEqual(params.GetString("url").c_str(),"DemoHandler");
		params.Dump();
	}

	TEST_METHOD(Http_Https_Server_Demo)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				string webRootFolder = "g:/test/web/";
				auto config = make_shared<tagWebServerConfig>();
				auto ajaxHandler = make_shared<AjaxCommandHandler>();
				AddChild(ajaxHandler);

				{
					auto jsonHandler = make_shared<JsonHandler>();
					AddChild(jsonHandler);
					config->mJsonHandler = jsonHandler;
				}


				config->mWebRootFolder = webRootFolder.c_str();
				config->mAjaxCommandHandler = ajaxHandler;

				{
					auto svr(make_shared<HttpServer>());
					svr->SetConfig(config);

					AddChild(svr);
					int ret = svr->StartServer(80);
				}

				{
					auto crtFile = "D:/bear/server/bin/bear.jjyip.com.p12";
					//auto crtFile = "D:/bear/server/bin/xwpcom.tpddns.cn.p12";//这个没备案，不能在微信小程序中使用
					TcpServer_Windows::InitSSL(crtFile);

					for(int i=0;i<2;i++)
					{
						auto svr(make_shared<HttpServer>());
						svr->SetConfig(config);

						svr->EnableTls();

						AddChild(svr);

						int port = i == 0 ? 443 : 8443;
						int ret = svr->StartServer(port);
					}
					
					{
						auto svr(make_shared<HttpServer>());
						svr->SetConfig(config);

						//svr->EnableTls();

						AddChild(svr);

						int port = 80;
						int ret = svr->StartServer(port);
					}

				}

				{
					class DelayExit :public Runnable
					{
						void Run()
						{
							Looper::CurrentLooper()->PostQuitMessage();
						}
					};

					//postDelayedRunnable(make_shared<DelayExit>(), 60 * 1000);
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
					class DemoHandler :public Handler
					{
						SUPER(Handler);

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

	TEST_METHOD(HttpsGet2)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<HttpGet>();
				AddChild(obj);
				obj->EnableTls();

				auto url = "taobao.com:443/favicon.ico";
				obj->SignalHttpGetAck.connect(this, &MainLooper::OnHttpGetAck);
				obj->Execute(url);

			}

			void OnHttpGetAck(HttpGet*, string& url, int error, ByteBuffer& box)
			{
				box.MakeSureEndWithNull();
				LogV(TAG, "%s",box.data());
				int x = 0;
			}

		};

		make_shared<MainLooper>()->StartRun();

	}

	TEST_METHOD(HttpsGet)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<HttpGet>();
				AddChild(obj);
				obj->EnableTls();

				auto url = "https://163.com/index.htm";
				obj->SignalHttpGetAck.connect(this, &MainLooper::OnHttpGetAck);
				obj->Execute(url);

				{
					class DelayExit :public Runnable
					{
						void Run()
						{
							Looper::CurrentLooper()->PostQuitMessage();
						}
					};

					postDelayedRunnable(make_shared<DelayExit>(), 5 * 1000);
				}
			}

			void OnHttpGetAck(HttpGet*, string& url, int error, ByteBuffer& box)
			{
				int x = 0;
			}

		};

		make_shared<MainLooper>()->StartRun();

	}

	TEST_METHOD(websocket)
	{
		//http://www.blue-zero.com/WebSocket/
		//注意要在路由器做端口映射，然后采用wan ip,不能使用lan/local ip
		//ws://219.133.68.41:8080
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

				int ret = svr->StartServer(8080);

				/*
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
				*/
			}
		};

		make_shared<MainLooper>()->StartRun();
	}


};


}
