# CoreLooper Timer����
CoreLooper����ʱ����ʵ�ָ����ܶ�ʱ��,����,���ٺ͵���timer����������O(1)���Ӷȡ�

ÿ��Looper������timer��������Ҫ������timer�߳�
## Timer�ӿ�
CoreLooper.Handler��������3���ӿ�ʵ��timer
```cpp
	virtual long SetTimer(long& timerId, UINT interval);
	virtual void KillTimer(long& timerId);
	virtual void OnTimer(long timerId);
```
���timerʱ��Ҫ���ǿ����к�����Handler��Ҫtimer,��ÿ��Handlerֻ�õ�������timer

timer����long��ʶ���涨0����Ч��timerId

interval��λ:����,����Ϊ1,��0ʱ�ڲ��Զ�תΪ1

SetTimerʱ�������timerId��Ч������ɾ����timer,Ȼ���ٴ����µ�timer

����ʱtimerIdΪ�´�����timer id,SetTimer����ֵ��timerId��ͬ

KillTimerɾ��ָ����timer id,����ʱtimerId����0

�ɹ�ɾ����timerId����0,ʧ��ʱtimerId����ԭֵ����

ֵ��ע�����Handler.postDelayedRunnable���ڲ�Ҳ�ǲ���timer��ʵ�ֵ�,��ԭ������:
```cpp
virtual LRESULT LOOPER_SAFE postDelayedRunnable(std::shared_ptr<Runnable> obj, UINT ms);
```
### Timerע������
- ͬһLooper��timer�����δ���ִ�еģ�������OnTimer��Ӧ�ÿ��ٴ�����ʱ���أ�����ᵼ������timer�ӳ١�  
timer���۾���1���룬ʵ��ȡ���ڸ��ɡ�  

- CoreLooper��֤��KillTimer(timerId)֮�󲻻��ٴ�����timerId��OnTimer
- ������OnTimer���������SetTimer,KillTimer,ȫ���ᰴ��������Ϊ����
- CoreLooperÿ�δ���timerʱΪ�����룬�����OnTimer�е���sendMessage,�ڵȴ�sendMessage���صĹ����в����ٴδ���looper��timer����.���Ӧ�����ⲻ����ΪCoreLooper�����ͽ�����ٴ���,����ȫ�첽�����һ��ҵ�����ռ�ýϳ�ʱ�䣬Ӧ�÷��ڵ�����looper��������

### Timer�÷�ʾ��
```cpp
class DemoHandler:public Handler
{
	SUPER(Handler)
	
	long mTimerTest=0;
	long mTimerKeepAlive=0;

	void OnCreate()
	{
		__super::OnCreate();

		SetTimer(mTimerTest,1000);
		SetTimer(mTimerKeepAlive,10*1000);
	}
	
	void OnTimer(long timerId)
	{
		//���ڱ�Handler���д�����timer,����Ҫת��__super,�����Ҫ��ʱreturn
		if(timerId == mTimerTest)
		{
			//...
			return;
		}
		else if(timerId == mTimerKeepAlive)
		{
			//...
			return;
		}
		
		//���е���ʱ��timerId�ǻ��ഴ����timer,Ҫת��__super��������
		//�����ܱ�֤���ܼ̳��˶��ټ���ÿ��Handler��������Ӧ����ֻ��Ҫ��Ӧ���Ѵ�����timer
		__super::OnTimer(timerId);
	}
};
```
����÷�:
- ÿ��timerId���������Ա��������
- ��OnTimer�����մ���Ƶ�ʴӸߵ����ж�timerId,�����ʱreturn
  ÿ��HandlerӦ��ֻ�����Լ�������timer,���������DemoHandler.mTimerKeepAlive�Ի�����˵��û������ģ����Բ���Ҫ����__super::OnTimer����������,ֱ��return���ɡ�

## ��Android Timer�Ƚ�
���û�п�Android�ĵ���ʵ����֤����������Android��ÿ��Timer���󶼲��õ�����thread����.

������android�ĵ��й���Timer������:

Corresponding to each Timer object is a single background thread that is used to execute all of the timer's tasks, sequentially.

ʵ�ⷢ��ȷʵ���,�����Ϊ���е����Ԥ�ϵģ�Ŀǰ��ûŪ���Ϊʲô�������

���˸о�Android.Handler.postDelayed��sendMessageDelayed�����϶�timer�������÷���

## ��Windows Timer�Ƚ�
Windows��SetTimerԭ������
```cpp
UINT_PTR SetTimer( HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc );
```
������һ������,nIDEvent��timerId����app���䣬MFC��װʱҲ�����������÷�����������SetTimerʱ���ܲ�֪��������ʹ������ЩtimerId,�ر����ڻ���Դ�벻���õ�����¡��������ǡ��ѡ���˺ͻ�����ͬ��timerId,��app��Ϊ��̫��Ԥ�ϡ�

Ϊ��������⣬CoreLooper�Ľ���취��timerId�ɿ�����Զ�����,��������ͻ��಻�õ���timerId��ͻ��

���⣬���Է���MFC app����ܴ���10000����Ч��timer,CoreLooperû��������ơ�

## ʱ���ֲο�
���Ͽ�Դ��ʱ���ִ���Ƚ϶࣬����linux kernel�е�Ҳ�ǲ������Ʒ����������˸о������������ʵ��:
https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/  

CoreLooper timer��ʱ����Դ����Ҫ��Դ�������Ŀ���ڴ˱�ʾ��л!



