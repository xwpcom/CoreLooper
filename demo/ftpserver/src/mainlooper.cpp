#include "stdafx.h"
#include "mainlooper.h"
#include "src/ftpserver.h"
using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;

#define LIB_ROOT "D:/corelooper/projects/bin/x64"
#pragma comment(lib,LIB_ROOT "/corelooperD.lib")

MainLooper::MainLooper()
{
}

void MainLooper::OnCreate()
{
	__super::OnCreate();

	auto config = make_shared<tagFtpServerConfig>();
	{
		auto& obj= config->mVirtualFolder;
		obj.AddMount("home", "e:/soft");
	}

	auto obj(make_shared<FtpServer>());
	obj->SetConfig(config);
	int port = 21;
	auto ret=obj->StartServer(port);
	ASSERT(ret == 0);

	AddChild(obj);

	/*
	{
		shared_ptr<tagWebServerConfig> config = make_shared<tagWebServerConfig>();
		mWebServerConfig = config;
		shared_ptr<AjaxCommandHandler> ajaxHandler = make_shared<AjaxCommandHandler>();
		config->mWebRootFolder = webRootFolder.c_str();
		config->mAjaxCommandHandler = ajaxHandler;
		{
			shared_ptr<VirtualFolder> vm = make_shared<VirtualFolder>();
			vm->AddMount("sd", GetMediaRootPath());
			config->mVirtualFolder = vm;

			config->mMediaRootPath = GetMediaRootPath().c_str();
		}

		for (int idx = 0; idx < 2; idx++)
		{
			auto svr(make_shared<BaseHttpServer>());
			svr->SetConfig(config);

			string name = "WebServer";
			int ret = svr->StartSvr(8080);
			AddChild(svr, name.c_str());
		}
	}
	//*/
}
