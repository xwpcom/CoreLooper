# ��OSԭ���߳�ͨѶ
CoreLooper���ɱ����Ҫ�Ͳ���ϵͳԭ���߳�ͨѶ�������г�һЩ���÷���

## main��ں���
��򵥵�main����Ӧ������
```cpp
int main(int argc,char* argv[])
{
	return make_shared<MainLooper>(argc, argv)->StartRun();
}
```
���˷ǳ�������main��дһ��Ѵ���

## looper��os raw threadͨѶ
��Looper�Ѿ����������������,os raw thread���԰�ȫ�ĵ��ñ�עΪLOOPER_SAFE�Ľӿ�,����
```cpp
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
```
���⣬�û����Ի�������LOOPER_SAFE�ӿڷ�װ�Զ���Ľӿ�,����Ҳ��LOOPER_SAFE�ġ�
### ����Looper::BindTLSLooper()
��os raw thread����LOOPER_SAFE�ӿ�ʱ����Щ����Ҫ�ȴ�CoreLooper�ظ��ģ������ڲ���ͨ��epoll/iocp/kevent�����ʵ�ֵ�,ÿ�ε���֮ǰCoreLooper���ڲ�����raw thread����epoll/iocp/kevent����Ա���ӦLooper�ظ����յ��ظ�֮�����Զ���raw thread���پ����  
���ż�����ã�����ûʲô����,�Ͼ�������������Щ����Ƿǳ���ġ�  
���os raw threadƵ���ĵ���CoreLooper�ӿڣ�����ÿ�ζ�Ҫ���´������Ȼ�������Ϲرգ������о����ܲ�����Ч��  
Ϊ��������⣬����ʹ��Looper::BindTLSLooper��raw��һ��TLS looper,CoreLooper��⵽����TLS looperʱ��ֱ��ʹ������
һ��������������һ��shared_ptr<Looper> mTlsLooper������OS UI�����������  
���Ҫ��OS����raw thread��Ƶ�����ã�Ҳ�����մ�������

��Windows MFCΪ��,��MFC CWinApp�����в���һ����Ա������shared_ptr<Looper> mTlsLooper  
��App::InitInstance��  
mTlsLooper=Looper::BindTLSLooper();  
���������ٵ���CoreLooper�ӿ�ʱ�Ͳ���ÿ�ζ�����������epoll/iocp/kevent����ˡ�

Looper��������Looperʱû��������⡣
