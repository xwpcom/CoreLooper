# �����
CoreLooper��ܲ������νṹ����������ʱ�����  
- ���ڵ���MainLooper  
- �ڵ������Handler��Looper����
- �ڵ������0������ӽڵ㡣  
- �ڵ㼶��û������

Ϊ����鿴���νṹ,libhttp�ṩ�������ӿ�tree.xml��proc.xml  
proc.xml��ֻչʾ�������ݵĽڵ㣬��������BindProcData  
tree.xmlչʾ���еĽڵ�

�Ե�Ԫ����Http.TreeΪ��
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
