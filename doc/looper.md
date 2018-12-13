# Looper
在CoreLooper框架中,Looper是Handler子类，便于统一管理普通Handler和Looper,这会带来很大的便利，并且在各种应用场合下，非常自然便利。这应该是本框架首创，目前在其他开源项目中都没有看到过类似的做法。

尽管Looper名字来源于Android.Looper,但Looper的设计主要是受到Windows的启发。Windows api PostThreadMessage表明thread也能响应消息。CoreLooper将这种思想发扬光大，Looper作为Handler的子类,支持Handler的所有接口和语义。
## Looper主要接口
在Handler的基础上,Looper主要增加了如下接口
```cpp
	virtual int Start();//在新的线程中运行
	virtual int StartRun();//在当前OS原生线程中直接运行,主要用于在main函数中
	virtual int  PostQuitMessage(int exitCode = 0);
	virtual int  GetQuitCode()const;

```
Looper有成员string mThreadName,调试时可以在linux ps,windows vs中看到线程名称。
### 创建Looper
```cpp
	auto looper = make_shared<WorkLooper>();
	AddChild(looper);
	looper->Start();
```
注意Looper.Start()并不一定会创建新的线程，它可能复用线程池中的线程。 
每个线程同一时刻只能只绑定一个Looper,当绑定的Looper析构之后，此线程才能绑定到新的Looper.

### 退出Looper
```cpp
	looper->PostQuitMessage();
```
looper收到quit消息时会递归通知其所有子Handler销毁，等所有子Handler都析构后才退出looper线程。

