#include "stdafx.h"
#include "mainlooper.h"
#include "src/ftpserver.h"
#include "libhttp/libhttp.inl"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;
using namespace Bear::Core::Net::Http;

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
		auto svr(make_shared<HttpServer>());
		svr->SetConfig(config);

		AddChild(svr);
		svr->StartServer(8080);
	}
}
