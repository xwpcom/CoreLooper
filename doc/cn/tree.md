# 组件树
CoreLooper框架采用树形结构来管理运行时组件。  
- 根节点是MainLooper  
- 节点可以是Handler和Looper子类
- 节点可以有0到多个子节点。  
- 节点级数没有限制

为方便查看树形结构,libhttp提供了两个接口tree.xml和proc.xml  
proc.xml是只展示绑定了数据的节点，即调用了BindProcData  
tree.xml展示所有的节点

以单元测试Http.Tree为例
```cpp
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

```
