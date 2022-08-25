#include "stdafx.h"
#include "libhttp.inl"
#include "libhttp/httpget.h"
#include "libhttp/httppost.h"

static const char* TAG = "Http";

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Http;

#ifdef _MSC_VER
#include "parser_unittest.h"
#include "libhttp/httpacker.h"
#include "string/stringparam.h"
#include "libhttp/telnet.h"

#include "CppUnitTest.h"
#define new DEBUG_NEW
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace Bear::Telnet;

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
		return ack;
	}
};

IMPLEMENT_AJAX_CLASS(Ajax_Info, "Info", "")

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

				//postDelayedRunnable(make_shared<DelayExitRunnable>(), 60 * 1000);
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

				postDelayedRunnable(make_shared<DelayExitRunnable>(), 30 * 1000);

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

			void OnHttpAck(Handler* handler, string&, int error, ByteBuffer&)
			{
				auto obj = dynamic_cast<HttpGet*>(handler);
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

				auto url = "https://gateway.shuiwutong.com.cn/flhiot/api/datacollect/http/haitian";
				obj->SignalHttpGetAck.connect(this, &MainLooper::OnHttpGetAck);
				obj->Execute(url);

			}

			void OnHttpGetAck(Handler* handler, string& url, int error, ByteBuffer& box)
			{
				auto obj = dynamic_cast<HttpGet*>(handler);

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
					postDelayedRunnable(make_shared<DelayExitRunnable>(), 5 * 1000);
				}
			}

			void OnHttpGetAck(Handler*, string& url, int error, ByteBuffer& box)
			{
				int x = 0;
			}

		};

		make_shared<MainLooper>()->StartRun();

	}
	
	TEST_METHOD(HttpGet_)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				auto obj = make_shared<HttpGet>();
				AddChild(obj);
				obj->SetHttpAction("POST");

				auto url = "http://cs.xqxyd.com/dimai.php";
				obj->SignalHttpGetAck.connect(this, &MainLooper::OnHttpGetAck);
				obj->Execute(url);

				{
					postDelayedRunnable(make_shared<DelayExitRunnable>(), 5 * 1000);
				}
			}

			void OnHttpGetAck(Handler*, string& url, int error, ByteBuffer& box)
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

				postDelayedRunnable(make_shared<DelayExitRunnable>(), 30 * 1000);
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(HttpAcker_)
	{
		{
			HttpAcker obj;
			obj.Parse(
				"HTTP/1.1 200 OK\r\n"
				"\r\n"
			);

			Assert::IsTrue(obj.version() == "HTTP/1.1");
			Assert::IsTrue(obj.code() == 200);
		}
		{
			HttpAcker obj;
			obj.Parse(
				"HTTP/1.1 200 OK\r\n"
				"Server: webserver\r\n"
				"Content-Type: application/xml\r\n"
				"Set-Cookie: WebSession_0eb6f4251b=2a65bbbaeb553b3a01a5e93c783d8fd2c67406a333eb6ffc83269ddd60843b98; path=/;HttpOnly\r\n"
				"\r\n"
			);

			Assert::IsTrue(obj.version() == "HTTP/1.1");
			Assert::IsTrue(obj.code() == 200);

			auto& fields = obj.fields();
			Assert::IsTrue(fields["Server"] == "webserver");
			Assert::IsTrue(fields["Content-Type"] == "application/xml");

			auto setCookie = fields["Set-Cookie"];
			LogV(TAG, "Set-Cookie=[%s]", setCookie.c_str());
			Assert::IsTrue(setCookie == "WebSession_0eb6f4251b=2a65bbbaeb553b3a01a5e93c783d8fd2c67406a333eb6ffc83269ddd60843b98; path=/;HttpOnly");

			{
				auto& items = StringParam::ParseItems(setCookie, "; ");
				for (auto& item : items)
				{
					LogV(TAG, "[%s]=[%s]", item.first.c_str(), item.second.c_str());
				}

				auto key = items["WebSession_0eb6f4251b"];
				Assert::IsTrue(key == "2a65bbbaeb553b3a01a5e93c783d8fd2c67406a333eb6ffc83269ddd60843b98");
			}
		}

		{
			HttpAcker obj;
			obj.Parse(
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 5\r\n"
				"\r\n"
				"Hello"
				"extraData"//故意增加其他数据
			);

			auto& body = obj.body();
			Assert::IsTrue(body == "Hello");
		}

		{
			HttpAcker obj;
			obj.Parse(
				"HTTP/1.1 200\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"74\r\n"
				"{\"access_token\":\"910c6a99-64bd-4bae-84d6-0eba452e5390\",\"token_type\":\"bearer\",\"expires_in\":6819,\"scope\":\"connon-api\"}\r\n"
				"0\r\n"
			);

			auto& body = obj.body();
			auto sz = body.c_str();
			Assert::IsTrue(body == "{\"access_token\":\"910c6a99-64bd-4bae-84d6-0eba452e5390\",\"token_type\":\"bearer\",\"expires_in\":6819,\"scope\":\"connon-api\"}");
			Assert::IsTrue(body.length() == 0x74);
		}
		{
			HttpAcker obj;
			obj.Parse(
				"HTTP/1.1 200\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"74\r\n"
				"{\"access_token\":\"910c6a99-64bd-4bae-84d6-0eba452e5390\",\"token_type\":\"bearer\",\"expires_in\":6819,\"scope\":\"connon-api\"}\r\n"
				"5\r\n"
				"Hello\r\n"
				"0\r\n"
			);

			auto& body = obj.body();
			auto sz = body.c_str();
			Assert::IsTrue(body == "{\"access_token\":\"910c6a99-64bd-4bae-84d6-0eba452e5390\",\"token_type\":\"bearer\",\"expires_in\":6819,\"scope\":\"connon-api\"}Hello");
			Assert::IsTrue(body.length() == 0x79);
		}
	}
	
	TEST_METHOD(StressTest)
	{
		/*
		2021.01.11
		最近发现iot经常crash在libhttp.dll,call stack耗尽，导致没法生成.dmp文件
		*/

		class MainLooper :public MainLooper_
		{
			long mTimer_Test=0;
			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimer_Test, 100);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_Test)
				{
					TestHttpPost();
					TestDownloadFile();
					return;
				}

				__super::OnTimer(id);
			}

			int TestHttpPost()
			{
				auto obj = make_shared<HttpPost>();
				AddChild(obj);
				obj->SetServerPort("192.168.1.3", 79);
				string url = StringTool::Format("/ISAPI/Security/sessionLogin?timeStamp=%lld", (LONGLONG)time(nullptr));
				obj->AddHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");

				{
					string user = "bear";
					string sn = "123";

					string text = "<SessionLogin>";
					XmlString x;
					x.AddItem("userName", user);
					x.AddItem("password", sn);
					text += x.GetString().c_str();
					text += "</SessionLogin>";
					ByteBuffer box;
					box.Write(text);

					obj->SetBodyRawData(box);
				}

				class PostAckHandler :public HttpPostAckHandler
				{
				public:
					shared_ptr< Handler> mRef;
					sigslot::signal2<int, const string&> SignalLoginAck;
					sigslot::signal1<HttpPostAckHandler*> SignalFinish;
					PostAckHandler()
					{
						LogV(TAG, "%s(%p)", __func__, this);
					}
					~PostAckHandler()
					{
						LogV(TAG, "%s(%p)", __func__, this);
					}
				protected:
					void OnPostAck(const string& ack)
					{
						LogV(TAG, "%s", ack.c_str());
						SignalLoginAck(0, ack);
					}

					void OnPostFail(int error, string desc)
					{
						LogW(TAG, "%s,desc=[%s]", __func__, desc.c_str());
						Destroy();
						SignalLoginAck(-1, "");
					}

					void Destroy()
					{
						SignalFinish(this);
					}
				};

				auto handler = make_shared<PostAckHandler>();
				handler->SignalLoginAck.connect(this, &MainLooper::OnLoginAck);
				handler->mRef = shared_from_this();
				obj->SetAckHandler(handler);
				obj->Start(url);
				return 0;
			}

			void OnLoginAck(int error, const string& ack)
			{

			}

			/* 下载超大文件，注意不要保存在ssd */
			void TestDownloadFile()
			{
				static int idx = -1;
				if (idx >= 2)
				{
					return;
				}

				++idx;

				auto obj(make_shared<HttpGet>());
				AddChild(obj);

				obj->SignalHttpGetAck.connect(this, &MainLooper::OnDownloadAck);

				string url = "http://192.168.1.3:79/historyDownload/office2007.zip";
				//url = "http://download.agora.io/sdk/release/Agora_Native_SDK_for_Windows_v2_2_1_FULL.zip";
				//url = "http://www.nsfocus.com.cn/";
				string saveAsFilePath = StringTool::Format("e:/test/%04d.zip",idx);
				obj->Execute(url, saveAsFilePath);
			}
			void OnDownloadAck(Handler* handler, string&, int error, ByteBuffer&)
			{
				auto obj = dynamic_cast<HttpGet*>(handler);
				if (error == 0)
				{
					LogV(TAG, "download success,speed=%.1f KB/S", obj->GetSpeed());	/* 本机约50MB/s */
				}
				else
				{
					LogW(TAG,"download fail,error=%d", error);
				}

			}

		};

		make_shared<MainLooper>()->StartRun();
	}

	TEST_METHOD(uploadFile)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				//string fileName = "2013.10.02.zip";
				//string filePath = "F:/Picture&Video/" + fileName;

				string fileName = "x.txt";
				string filePath = "d:/" + fileName;

				auto obj = make_shared<HttpPost>();
				AddChild(obj);

				auto t = ShellTool::GetCurrentTimeMs();
				auto time = StringTool::Format("%04d.%02d.%02d_%02d.%02d.%02d"
					, t.year
					, t.month
					, t.day
					, t.hour
					, t.minute
					, t.second
				);

				string noise;
				noise = StringTool::Format("%d", 123);
				string uid = "00000EMU";

				obj->SetServerPort(
					"iot.jjyip.com"
					, 80);
				obj->AddFile("logFile", filePath);

				class AckHandler :public HttpPostAckHandler
				{
					void OnPostAck(const string& ack)
					{
						LogV(TAG, "%s,ack=[%s]", __func__, ack.c_str());
						OnAck(0);
					}

					void OnPostFail(int error, string desc)
					{
						LogV(TAG, "%s,desc=[%s]", __func__, desc.c_str());
						OnAck(error);
					}

					void OnAck(int error)
					{
						LogV(TAG, error == 0 ? u8"上传文件成功" : u8"上传文件失败");

						GetMainLooper()->PostQuitMessage();
					}

				public:
					weak_ptr<Handler> mManager;
				};

				auto handler = make_shared<AckHandler>();
				handler->mManager = shared_from_this();
				obj->SetAckHandler(handler);

				obj->Start("/uploadFile?tag=mcuLog&uid=00000EMU&fileName=" + fileName);
				//obj->Start("/uploadFile?tag=mcuLog&uid=00000EMU");
			}
		};

		make_shared<MainLooper>()->StartRun();
	}

};


TEST_CLASS(Telnet_)
{
	TEST_METHOD(telnetConnect)
	{
		class MainLooper :public MainLooper_
		{
			void OnCreate()
			{
				__super::OnCreate();

				/*
				auto obj = make_shared<TelnetClient>();
				AddChild(obj);

				Bundle bundle;
				bundle.Set("address", "192.160.1.101");
				bundle.Set("port", 23);
				obj->StartConnect(bundle);
				*/
			}
		};

		make_shared<MainLooper>()->StartRun();
	}
};

}
#else

using namespace Bear::Core;
int main()
{
	LogV(TAG, "%s",__func__);

	class MainLooper :public MainLooper_
	{
		SUPER(MainLooper_);
		int mCount = 1;
		int mAcked = 0;

		void OnCreate()
		{
			__super::OnCreate();

			if (0)
			{
				string url = "https://163.com/index.htm";//pclinux test ok
				url = "https://iot.jjyip.com/reportDataEx.json?hello";//test fail
				auto obj = make_shared<HttpPost>();
				obj->EnableVerbose();
				obj->EnableTls();
				AddChild(obj);
				string body = "{}";

				obj->AddHeader("Content-Type", "application/json");
				obj->SetBody(body);

				obj->Start(url, [this](const string& url, int error, const string& ack)
					{
						LogI(TAG, "%s", ack.c_str());
						PostQuitMessage();
					}
				);

			}
			else
			{
				for (int i = 0; i < mCount; i++)
				{
					auto obj = make_shared<HttpGet>();
					AddChild(obj);
					obj->EnableTls();
					obj->EnableVerbose();
					obj->SetHttpAction("POST");

					string body = "{}";

					obj->AddHeader("Content-Type", "application/json");
					obj->SetBody(body);

					//auto url = "https://163.com/index.htm";
					auto url = "https://iot.jjyip.com/index.htm";
					obj->SignalHttpGetAck.connect(this, &MainLooper::OnHttpGetAck);
					obj->Execute(url);
				}
			}

			DelayExit(120*1000);
		}

		void OnHttpGetAck(Handler*, string& url, int error, ByteBuffer& box)
		{
			LogV(TAG, "%s,error=%d,data=[%s]",__func__,error,box.data());

			++mAcked;
			LogV(TAG, "recv acked count=%d", mAcked);
			if (mAcked == mCount)
			{
				PostQuitMessage();
			}
		}

		};

	make_shared<MainLooper>()->StartRun();
	return 0;
}
#endif

