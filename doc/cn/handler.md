# Handler
Handler��CoreLooper��ܹ���Ļ�������������Android�е�Handler,MFC�е�CWnd��win32�е�HWND
Handler����ʹ��timer,��ͬһlooper�Ϳ�looper sendMessage����postMessage,�충����Handler����Ϣ

## Handler��������

![img](images/handler.lifecycle.png)


### ����Handler
��������ʽ������Handler,
- ��һ����c++���Բ����ϵĴ���,��new����
  Handler������heap�д�����������stack�д���
  ǿ�ҽ������make_shared������c++���󣬲�Ҫֱ�ӵ���new,һ��newҲ��Ҫ��Ҳ��Ҫ����delete,����ȫ������smart pointer������

- �ڶ����ǿ�ܲ����ϵĴ���,���󶨵�Looper������parent/child��ϵ
 Handlerֻ����Looper�����´���,���ұ���󶨵�Looper���������ʹ��
��ܽӿ�����
```cpp
virtual void LOOPER_SAFE Create(shared_ptr<Handler> parent);
virtual int LOOPER_SAFE AddChild(weak_ptr<Handler> child, string name = "");
```
�����¼��ֵ����÷�
```cpp
auto obj=make_shared<Handler>();
//��ʱHandler��û�а󶨵�Looper,����Handler������ߴ˴�����ʼ������
obj.Create(parent);
```
���߸���һ�㣬�ڲ���Ҫ��ʼ��ʱ
```cpp
parent.AddChild(make_shared<Handler>());
```
������ڸ����е���(һ�����ڸ���.OnCreate)������Լ�Ϊ
```cpp
AddChild(make_shared<Handler>());
```
˵��:
- Handler֧�ֿ�looper����,������MainLooper�д���HandlerȻ��󶨵�WorkLooper��
- ֻ�а󶨵�Looper֮��,handler����ʹ�ÿ���ṩ�Ĺ��ܣ�����timer,sendMessage,postMessage��


### ����Handler
```cpp
virtual void LOOPER_SAFE Destroy();
```
����Handler.Destroy()������Handler,����Handler�󶨵�Looper�е���OnDestroy()
app���Զ�ε���Handler.Destroy()����������������Ӱ�죬�ɿ�ܱ�֤OnDestroy()�ᱻ���ò���ֻ������һ��

���ٺ������ǲ�ͬ�ĸ���
�����Ǳ���ܶ���ģ�������c++���Զ����

˵��:
- ���ٲ�Ӱ��timer,sendMessage��postMessage�ȹ��ܣ���Щ���������ٺ���Ȼ����
- ���ٺ��Handler�����ٴ�Create����AddChild
- parent����ʱ���Զ��ݹ����������е�child,��������
- parent������ʱ�����������AddChild,��ʱchild�������������������Զ�����

### ����
��Handler�����һ��shared_ptrʧЧʱ���������

#### Handler�����������ĸ�Looper������
- ���ֻ�ڰ󶨵�Looper��ʹ��Handler,����ڴ�Looper������Handler
- �����Looperʹ����һLooper�е�shared_ptr< Handler>,������Handler.Destroy()����Ȼ���ִ�shared_ptr,���кܵ͵ļ��ʿ�Looper����Handler
#### ��Looper����Handler���ͽ���취
������Handler.Destroy()��,��ԭ��Looper����һ��shared_ptr< Handler>���ӵ�gc(��������)�������ϼ����ա�
���⣬��gc�б�����ĿʱLooper�ᶨʱ�����ա�

�����㷨����
1. ����weak_ptr���ô�Handler
2. ���shared_ptr< Handler>
3. ��weak_ptr����lock(),���Ϊnullptr,˵��Handler������;�����Ϊnullptr,˵�����������,HandlerҪ���¼ӵ�gc�б�

�ٶ�:
LooperA��gc��������HandlerA,
 LooperB��shared_ptrҲ������HandlerA
��LooperA�����������2��,����û���е�3����
��ʱ��LooperB����������õ�shared_ptr< HandlerA>,�����LooperB������HandlerA

���ܼ��ʺ�С����������ڿ�ܲ����c++���Բ��涼û��������  
��õĽ���취����app��֤�Զ����Handler������֧�ֿ�looper������
��ܱ����Hanlder�ǿ�Looper������ȫ�ġ�

 ����: ** todo ** ���Ҫ�ṩ�ӿ�������app���Կ�Looper����Handler�����Ӱ�ȫ�ԡ�

####  �����ܽ�
��ܻ����ɱ�֤�ڰ󶨵�Looper������Handler(�����������)
��ܱ�֤Handler�������looper�����ǰ�ȫ��,app�Զ����Handler����ֻ����app���б�֤
����취�ܼ�,Handler.OnDestroy()ʼ�����ڰ󶨵�Looper�е��õ�,Handler����ֻ��Ҫ��OnDestroy()�������ɡ�

## Handler�ṹ��
Handler�ṹ��������ͼ�е�Windows������

![img](../images/windows.tree.png)

����ͼ��ʾ,SPY++�����οؼ���ÿ���ڵ㶼��һ��HWND����,���ڵ������洰��  
��CoreLooper��Ҳ�����ƵĽṹ�������ڵ���MainLooper  
parent.AddChild��obj.Create���ǰѽڵ���ص��ϼ��ڵ�,������CoreLooper��Looper��Handler���࣬���Խڵ��������ͨ��Handler,Ҳ������Looper��  

�������νṹ���������ж����кܶ�ô�
todo:�ڵ��÷�,ajax,proc,shortcut

## ��ʱ�� Timer
``` cpp
	virtual long SetTimer(long& timerId, UINT interval);
	virtual long KillTimer(long& timerId);
	virtual void OnTimer(long timerId);
```

����ڲ�����ʱ���ֹ���timer
���������ٺʹ���timer�ĸ��Ӷȿ���Ϊ��O(1),�����ɹ��������timer

### SetTimer
�ڵ���Handler.Create����.AddChild����Handler֮�󣬾Ϳ��Ե���SetTimer��������ʱ��
����:
- timerId:����&�������,0Ϊ��Чֵ,�������0��timerIdʱ,SetTimer�������ٴ�timer,�ٴ����������µ�timerId
- intervalΪ�����������ʾ���ʱ�䣬��λ:����,��С����Ϊ1ms,��0ʱ��Ĭ��ȡ1ms,ʵ�ʴ���OnTimer�ľ���ȡ����OS

### KillTimer
��������Ҫ��ʱ��ʱ���ɵ���KillTimer������

Timer�Ĳ���˵��:
- ��Handler����ʱ���Զ����ٴ�Handler���������ж�ʱ��
- Handler.Destroy()��Ӱ��timer

### OnTimer
```cpp
virtual void OnTimer(long timerId);
```

## ����LOOPER_SAFE
����LOOPER_SAFE���εĽӿڿ��԰�ȫ�Ŀ�looper����
û�в���LOOPER_SAFE���εĽӿڣ�����֤��looper��ȫ���ã�Ӧ��ֻ��handler����looper�����


## ����SUPER��__super
__super��Windows VC++��չ�Ĺؼ��֣��ܷܺ���ĵ��ø��෽����ʹ�÷ǳ����㡣  
��������������֧��__super,����CoreLooper���������SUPER�������Ӵ˹��ܡ�

�����÷�
```cpp
		class MainLooper :public MainLooper_
		{
			SUPER(MainLooper_)
			void OnCreate()
			{
				__super::OnCreate();
			}
		};

```
�ô��ǿ���������ƽ̨��֧��__super::OnCreate�����ĵ��÷�����


# Handler�ӿ�
```cpp
	virtual void LOOPER_SAFE Create(shared_ptr<Handler> parent);
	virtual void LOOPER_SAFE Destroy();

	//windows style
	virtual LRESULT LOOPER_SAFE sendMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);
	virtual LRESULT LOOPER_SAFE postMessage(UINT msg, WPARAM wp = NULL, LPARAM lp = NULL);

	//android style
	virtual LRESULT LOOPER_SAFE sendRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postDelayedRunnable(shared_ptr<Runnable> obj, UINT ms);
	virtual void    LOOPER_SAFE cancelRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE sendMessage(shared_ptr<Message> message);
	virtual LRESULT LOOPER_SAFE postMessage(shared_ptr<Message> message);

	virtual long SetTimer(long& timerId, UINT interval);
	virtual void KillTimer(long& timerId);

	virtual int LOOPER_SAFE AddChild(weak_ptr<Handler> child, string name = "");
	shared_ptr<Handler> LOOPER_SAFE GetParent()const;
	string LOOPER_SAFE GetUrl()const;
	virtual LONG_PTR LOOPER_SAFE GetId()const;
	virtual void LOOPER_SAFE SetId(LONG_PTR id);

	string LOOPER_SAFE GetObjectName()const;
	void LOOPER_SAFE SetObjectName(const string& name);

	bool LOOPER_SAFE IsLooper()const;
	virtual shared_ptr<Handler> LOOPER_SAFE FindObject(string url);
	virtual shared_ptr<Handler> LOOPER_SAFE GetChild(LONG_PTR id);
	
	int LOOPER_SAFE RegisterShortcut(const string& name, weak_ptr<Handler> obj);
	shared_ptr<Handler> LOOPER_SAFE Shortcut(const string& name);



```

## ��
Ϊ��ַ���CoreLooper��ܵ�������Handler�ڴ�����Ӧ����ӽṹ������,���ǰ����Ϊ�󶨡�



##  �������ڹ���
��c++���й��캯������������������£�Ϊ��ͳһ�ͼ򻯿�����̣�CoreLooper����������Create,Destroy��������ܲ����״̬����������Handler������������ڡ�����˵�����Ǿ��������ʽ���������,����Windows.MFC,ios.objectc,ios.swift,android�ӿ����洦�ɼ���

��˵QT�Ķ����ڹ��캯���оͿ��Ե���SetTimer�Ƚӿڣ����о��ȽϺ��á�Ϊ�ˣ������CoreLooperʱ���������϶�ʱ���о������շ��ֲ�̫�ã���Ҫ��ʵ�ֵĲ������Ȼ���Ƚϱ�Ť����CoreLooper��������ͻ��������һЩ�ط�����������ȱ�ݣ�����˵�����ڹ��캯������û������������麯����

����������CoreLooper���õ�������ʽ���죬���캯����ֻ��������ĳ�ʼ��,������Create�ӿڣ����ᴥ��Handler.OnCreate,�����Ƚ�"��"�Ĺ��ܶ�Ӧ����OnCreate��������

### Create
�����÷�����parent��OnCreate()�е������´���
```cpp
	auto obj=make_shared<DemoHandler>();
	obj->Create(parent);
```
���߸��򵥣�
```cpp
	AddChild(make_shared<DemoHandler>());
```
AddChild�ڲ����Ⲣ����Ҫʱ�Զ�����obj->Create()

Create()�ᴥ��OnCreate()
��ܱ�֤����Handlerԭ��Looper�е���Handler.OnCreate,���ҽ�����һ��



### Destroy

Destroy()�ᴥ��OnDestroy()
��ܱ�֤����Handlerԭ��Looper�е���Handler.OnDestroy,���ҽ�����һ��

## ����shared_ptrѭ������

��CoreLooper����ڲ�,parent��child��weak_ptr,child��parent��shared_ptr

��ʱ�������ܣ�ʹ�ñ�ݵ�ԭ��parent���ܻ���child��shared_ptr,�Ӷ��γ�shared_ptrѭ�����á�����ʹ����ȫ������Ҳ�������ֻ��Ҫע��һ��,��parent OnDestroy��Ҫ�ǵ������child��shared_ptr��

��ʱͬ��handler֮��Ҳ�������÷�������취��һ��

�˳����������������  
��OnCreate()�л���shared_ptr  
��OnDestroy()�����shared_ptr


ǰ������,��һ��С����Ҫע�⡣ϸ�ڷ�������:
����Destroy()֮��CoreLooper��֤���ϵ��ò�������һ��OnDestroy(),  
��ʱ�����app��û�жԴ�Handler��shared_ptr,��CoreLooper��������Handler  
���app�����л��д�Handler��shared_ptr,CoreLooper��⵽������������Handler,������ӵ�gc�����в���ʱ��������,����������֮ǰ,��handler�Ŀ�ܹ��ܣ�����timer,message�������������ģ�����ڴ˽׶ι���ѭ�����ã�ֻ����app���д����ˡ�
��õİ취�ǻرܣ������Ǳ�֤����OnCreate()�г���ѭ�����á�

