# 和OS原生线程通讯
CoreLooper不可避免的要和操作系统原生线程通讯，本文列出一些常用方法

## main入口函数
最简单的main函数应该如下
```cpp
int main()
{
	auto looper = make_shared<MainLooper>();
	looper->StartRun();
	return looper->GetQuitCode();
}
```
如果需要处理main参数,可以如下
```cpp
int main(int argc,char* argv[])
{
	auto obj = make_shared<MainLooper>();
	obj->ParseCommandLine(argc, argv);

	obj->StartRun();
	return obj->GetQuitCode();
}
```
个人非常反感在main中写一大堆代码

## looper和os raw thread通讯
在Looper已经运行起来的情况下,os raw thread可以安全的调用标注为LOOPER_SAFE的接口,比如
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
另外，用户可以基于如上LOOPER_SAFE接口封装自定义的接口,这样也是LOOPER_SAFE的。
### 关于Looper::BindTLSLooper()
当os raw thread调用LOOPER_SAFE接口时，有些是需要等待CoreLooper回复的，这在内部是通过epoll/iocp/kevent句柄来实现的,每次调用之前CoreLooper在内部会替raw thread创建epoll/iocp/kevent句柄以便响应Looper回复，收到回复之后再自动替raw thread销毁句柄。  
如果偶尔调用，这样没什么问题,毕竟创建和销毁这些句柄是非常快的。  
如果os raw thread频繁的调用CoreLooper接口，由于每次都要重新创建句柄然后又马上关闭，这样感觉性能不够高效。  
为解决此问题，可以使用Looper::BindTLSLooper给raw绑定一个TLS looper,CoreLooper检测到存在TLS looper时可直接使用它。
一般做法如下是用一个shared_ptr<Looper> mTlsLooper缓存在OS UI框架主对象中  
如果要在OS其他raw thread中频繁调用，也可以照此做法。

以Windows MFC为例,在MFC CWinApp对象中采用一个成员来缓存shared_ptr<Looper> mTlsLooper  
在App::InitInstance中  
mTlsLooper=Looper::BindTLSLooper();  
这样后面再调用CoreLooper接口时就不用每次都创建和销毁epoll/iocp/kevent句柄了。

Looper调用其他Looper时没有这个问题。
