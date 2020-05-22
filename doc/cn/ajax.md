# AJAX
CoreLooper����AJAX���������չʾapp����ʱ��Handler�ṹ��  
ÿ��Handler���ܰ���Ҫչʾ��proc.xml�е�����   

Ч��ͼ����

![img](images/proc.png)

�����ʱ�䣬��������������ҳ�����õ�չʾ
����ڿ���������˵�Ǻܷ����  
��ʽ��ӪʱҲ����ʹ�á�  
url�е�user,password������ʾ����ʽʹ��ʱ��ע��ӿڰ�ȫ��

## AjaxHandler
CoreLooper�ṩ��AjaxHandler�������ܷ������չajaxָ�����proc.xml��ʵ������:
- .h�ļ�
```cpp
class Ajax_Proc :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_Proc)
	string Process(const NameValue& params);
};
```

- .cpp�ļ�

```cpp
IMPLEMENT_AJAX_CLASS(Ajax_Proc, "proc", "DeviceConfig")

string Ajax_Proc::Process(const NameValue& params)
{
	string xml;
	LooperImpl* looper = Looper::GetMainLooper();
	if (!looper)
	{
		looper = Looper::CurrentLooper();
	}

	auto url = params.GetString("url");
	if (!url.empty())
	{
		auto obj = looper->FindObject(url);
		if (obj)
		{
			obj->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml);
		}
	}
	else
	{
		looper->sendMessage(BM_DUMP_PROC_DATA, (WPARAM)&xml);
	}

	string ack=StringTool::Format("<Result><Error>0</Error>%s</Result>"
		,xml.c_str()
	);

	return ack;
}

```

## �Զ���AJAXָ��
�ɲ��������Ajax_Proc������CoreLooper��Handler,Looper��ʵ���κι��ܡ�  
����˵���������url�п��Զ�λָ����Handler,looper�����Դ����������ִ��,չʾ���ݣ��޸����ݵȡ�  
Ҳ����ʵ�ִ�ͳ��cgi����Ĺ��ܡ�

����ϸ���뿴demo����
