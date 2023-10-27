#include "stdafx.h"
#include "mainlooper.h"
#include "src/ftpserver.h"
#include "libhttp/libhttp.inl"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;
using namespace Bear::Core::Net::Http;

class Ajax_Test :public AjaxHandler
{
	SUPER(AjaxHandler);
	DECLARE_AJAX_CLASS(Ajax_Test);
	string mTag = "test";
public:

	string Process(const NameValue& params)
	{
		int ret = 0;
		DynamicJsonBuffer jBuffer;
		auto& json = jBuffer.createObject();

		{
			auto uid = params.GetString("uid");
			auto tag = params.GetString("tag");
			LogV(mTag, "");

			json["error"] = ret;
			if (ret == 0)
			{
				json["uid"] = uid;
				json["tag"] = tag;
			}
		}

		string ack;
		json.printTo(ack);
		return ack;
	}
};
IMPLEMENT_AJAX_CLASS(Ajax_Test, "test.json", "")

MainLooper::MainLooper()
{
}

void MainLooper::OnCreate()
{
	__super::OnCreate();

	{
		auto config = make_shared<tagFtpServerConfig>();
		{
			auto& obj = config->mVirtualFolder;
			obj.AddMount("home", "g:/soft");
		}

		auto obj(make_shared<FtpServer>());
		AddChild(obj);

		obj->SetConfig(config);
		int port = 21;
		auto ret = obj->StartServer(port);
		ASSERT(ret == 0);
	}



	{
		//for demo

		auto config = make_shared<tagWebServerConfig>();
		config->mWebRootFolder = "d:/test/webRoot/";

		auto ajaxHandler = make_shared<AjaxCommandHandler>();
		AddChild(ajaxHandler);
		config->mAjaxCommandHandler = ajaxHandler;
		{
			auto jsonHandler = make_shared<JsonHandler>();
			AddChild(jsonHandler);
			config->mJsonHandler = jsonHandler;
		}

		auto svr(make_shared<HttpServer>());
		svr->SetConfig(config);

		AddChild(svr);
		svr->StartServer(8080);
	}
}
