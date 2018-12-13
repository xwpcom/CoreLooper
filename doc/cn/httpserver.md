# HttpServer

为了在浏览器中展示结构树和使用ajax子框架，CoreLooper内置了一个简单的HttpServer,主要功能如下

- 提供常用网页文件服务
- 支持HTTP POST上传文件
- 支持断点续传
- 支持ajax子框架，能很方便的自行扩展ajax指令

## 用法demo

```cpp
	auto svr(make_shared<HttpServer>());
	AddChild(svr);
	{
		auto config = make_shared<tagWebServerConfig>();
		config->mWebRootFolder = ShellTool::GetAppPath()+"/web";
		auto ajaxHandler = make_shared<AjaxCommandHandler>();
		config->mAjaxCommandHandler = ajaxHandler;
		svr->SetConfig(config);
		svr->AddChild(ajaxHandler);
	}

	int ret = svr->StartServer(80);

```

