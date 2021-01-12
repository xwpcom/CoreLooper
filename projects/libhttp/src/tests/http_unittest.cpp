#include "stdafx.h"
#include "CppUnitTest.h"
#include "libhttp.inl"
#include "libhttp/httpget.h"
#include "parser_unittest.h"
#include "libhttp/httpacker.h"
#include "string/stringparam.h"
#include "libhttp/httppost.h"

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
		
		//map<string,string> //��map��ʵ�ִ�Сд�޹����д�����
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
					//auto crtFile = "D:/bear/server/bin/xwpcom.tpddns.cn.p12";//���û������������΢��С������ʹ��
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
		//����http�����ٶ�,�����������豸�͵���֮������������ٶȣ��ȶ��Ե�ָ��
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

				auto url = "taobao.com:443/favicon.ico";
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

	TEST_METHOD(websocket)
	{
		//http://www.blue-zero.com/WebSocket/
		//ע��Ҫ��·�������˿�ӳ�䣬Ȼ�����wan ip,����ʹ��lan/local ip
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
				"extraData"//����������������
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
		�������iot����crash��libhttp.dll,call stack�ľ�������û������.dmp�ļ�
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

			/* ���س����ļ���ע�ⲻҪ������ssd */
			void TestDownloadFile()
			{
				static int idx = -1;
				if (idx >= 0)
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
					LogV(TAG, "download success,speed=%.1f KB/S", obj->GetSpeed());	/* ����Լ50MB/s */
				}
				else
				{
					LogW(TAG,"download fail,error=%d", error);
				}

			}

		};

		make_shared<MainLooper>()->StartRun();
	}
};


}
