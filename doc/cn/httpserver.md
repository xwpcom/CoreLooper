# HttpServer

Ϊ�����������չʾ�ṹ����ʹ��ajax�ӿ�ܣ�CoreLooper������һ���򵥵�HttpServer,��Ҫ��������

- �ṩ������ҳ�ļ�����
- ֧��HTTP POST�ϴ��ļ�
- ֧�ֶϵ�����
- ֧��ajax�ӿ�ܣ��ܷܺ����������չajaxָ��

## �÷�demo

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

