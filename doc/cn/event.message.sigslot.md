# Event,Message and sigslot �¼�,��Ϣ���źŲ�
�¼�����Ϣ�����߻���һ�����ƽ��ź�(����ָlinux�е�signal,��������QT�е��ź�)���Ǻ����׻����ĸ����Ҳ������⣬�ϸ�������Щ�����ʵ����˵��û��̫�������,���ǵı��ʶ������ݵ��ô���,����ʵ��Ϊ����̬�ȣ����Ķ���Щ���ﲻ���������֡�

## ��Ϣ������

CoreLooper����Android��WindowsӰ�죬����ͬʱ����ʵ�������ǵ���Ϣ�ӿڣ�����Handler����ʵ�ֵġ�����Looper��Handler���࣬������Щ�ӿ�Ҳ������Looper��  

```cpp
//windows��Ϣ�ṹ��Ҫ��Ա����
struct  tagMSG
{
UINT msg;
WPARAM wp;
LPARAM lp
//...
};
```
msg�Ǹ��޷�������,������ʶ��Ϣ����wp��lp�ֱ�����ָ�����͵Ĳ��������Դ��������ߵ�ַ��
����ṹ�ܼ򵥣�����CoreLooperû�ٷ�װ�������ڽӿ���ֱ�Ӵ���3��������  

���︽��˵һ�£�������Щ��˵Windows���Ǻ����⣬����C�����Դ����������ͺܷ��㣬��Ҫ��һЩĪ�������UINT,WPARAM��LPARAM֮������ͺꡣ���ҿ��������Windows�ľ���֮��,ͨ����Щ�꣬Windows API��16bit,32bit��64bitʱ��һ·����������30�������APIһֱ����ԭ�Ͳ��䡣��Щ�˲�������ʷԭ���ʵ�������һζָ��,�������������������̫������?  

```cpp
//��android message
class Message:public Object
{
public:
	LONGLONG	what = 0;
	LONG_PTR	arg1 = 0;
	LONG_PTR	arg2 = 0;
	shared_ptr<Bundle> mBundle;
	shared_ptr<Object> mObject;
};
```
what����windows��Ϣ�е�msg  
arg1��arg2����widnows��Ϣ�е�wp��lp  
��Ϊ����ʱ�е�OS,android����Ϣ����ͨ��Bundle������������������Դ���Objecvt����  

��������android��Ϣ����ǿ��һЩ������ֻ�Ǳ���CoreLooper�ڲ��ǲ�����Windows��Ϣ��װ��ʵ����android message�� 

���⣬java���и������õ�Runnable������Ҳ���뵽CoreLooper�С�


## ͬ��������Ϣ
ͬ����ָ������Ϣ��Ҫ�ȴ����շ���Ӧ��������Ϣ��Ȼ��ŷ��ط��ͷ�
```cpp
	//windows style
	virtual LRESULT LOOPER_SAFE sendMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);

	//android style
	virtual LRESULT LOOPER_SAFE sendRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE sendMessage(shared_ptr<Message> message);
	//ע��android sendMessage���첽�ģ������ȴ���Ӧ�ͷ���,����Ƿ�ֱ���ģ�����CoreLooper�´˽ӿ���ͬ���ģ���Ҫ�ȴ���Ӧ�ŷ���
	
```

ֵ��˵�����ǣ��ڵȴ����շ���Ӧ��Ϣ�ڼ䣬���ͷ�����Looper��Ȼ����Ӧ����Looper��������Ϣ�����Ǹ��ܹؼ��Ĺ��ܣ�����������������������:  
LooperA����Ϣmessage1��LooperB  
LooperB�ڴ�����Ϣmessage1�ڼ�Ҫ����Ϣmessage2��LooperA  
����ʱLooperA�ڵȴ�LooperB��message1�Ļظ�  
���LooperA�ڵȴ�ʱ������Ӧ���յ�����Ϣ������������ˡ�

CoreLooper���ڶ๦���У�������������ҵ�ʱ������ġ�ֱ��2016�����liblingle���ҵ�����취��ԭ����һ��bool�������ܸ㶨��,���Ǵ���������������о�liblingle,CoreLooper������ǰ�ü���������


## �첽Ͷ����Ϣ

�첽Ͷ����ָ������Ϣ�󣬲���Ҫ�ȴ����շ���Ӧ�����ͷ�ֱ��ִ�к���ָ�  


```cpp
//windows style
virtual LRESULT LOOPER_SAFE postMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
```

```cpp
//android style
	virtual LRESULT LOOPER_SAFE postMessage(shared_ptr<Message> message);
	virtual LRESULT LOOPER_SAFE postRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postDelayedRunnable(shared_ptr<Runnable> obj, UINT ms);

```

## sigslot�źŲ�(�¼������봥��)

sigslot�Ǹ��ܺ��õ��źŲۻ��ơ�  

sigslot������֧�ֶ��̰߳�ȫ�ģ���CoreLooperֻʹ�������еĵ��̣߳�ԭ����Ҫ��: sigslot�Ǽ�����Handler��ģ������ÿ��Handler����һ����,������������CoreLooper��ԭ��

sigslot֧��0��8���������ֱ���signal0,signal1...signal8  
�ֲ��õ�Ԫ��������ʾ�¼������ʹ���  

```cpp
//TEST_CLASS(Sigslot_).TEST_METHOD(Base)
class EventSource :public Handler
{
	SUPER(Handler)
public:
	sigslot::signal3<Handler*, const string&, int> SignalSomeEvent;

protected:
	void OnCreate()
	{
		__super::OnCreate();

		class Worker :public Runnable
		{
		public:
			weak_ptr<EventSource> mObject;
		protected:
			void Run()
			{
				auto obj = mObject.lock();
				if (obj)
				{
					obj->SignalSomeEvent(obj.get(), "hello", 2019);
				}
			}
		};

		auto obj = make_shared<Worker>();
		obj->mObject = dynamic_pointer_cast<EventSource>(shared_from_this());
		postDelayedRunnable(obj, 1000);
	}

};
class EventListener :public Handler
{
	SUPER(Handler)
public:
	void OnSomeEvent(Handler*, const string& msg, int value)
	{
		DW("%s,msg=%s,value=%d", __func__, msg.c_str(), value);

		Looper::CurrentLooper()->PostQuitMessage(0);
	}
};

class MainLooper :public MainLooper_
{
	SUPER(MainLooper_);

	void OnCreate()
	{
		__super::OnCreate();

		auto source = make_shared<EventSource>();
		AddChild(source);

		auto listener = make_shared<EventListener>();
		AddChild(listener);

		source->SignalSomeEvent.connect(listener.get(), &EventListener::OnSomeEvent);
	}
};
make_shared<MainLooper>()->StartRun();
```

��Ҫ˵������:  
- EventSource�ṩSignalSomeEvent�¼�Դ��ԭ����void (Handler*, const string&, int)  
ע��sigslot�¼���Ӧԭ�͵ķ���ֵ����void,������Ҫ������ͨ����������������  
- EventListener ��EventSource�¼�Դ��ʵ�ֺ���	void OnSomeEvent(Handler*, const string& msg, int value)
- �충source->SignalSomeEvent.connect(listener.get(), &EventListener::OnSomeEvent);
- ���� obj->SignalSomeEvent(obj.get(), "hello", 2019);
- �����¼�ʱ��sigslot�����EventListener�еĺ���

ע��sigslot���¼�Դ���¼���Ӧ�����Ƕ�Զ�Ĺ�ϵ��   ��һ���¼�Դ�ɶ�α�������ͬһlistenerҲ����������¼�Դ��

ִ�к�DT��ʾ����  
![img](../images/sigslot.base.png)
