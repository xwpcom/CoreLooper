# Runnable接口
如您所见，CoreLooper.Runnable与Android.Runnable基本相同(其实是Java.Runnable)  
Java中有些思想是值得cpp借鉴的。  
CoreLooper.Runnalbe接口如下

```cpp
class Runnable :public std::enable_shared_from_this<Runnable>
{
public:
	virtual ~Runnable() {}

	virtual void Run() = 0;
};

```
Handler中与Runnable相关的接口
```cpp
	virtual LRESULT LOOPER_SAFE sendRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postRunnable(shared_ptr<Runnable> obj);
	virtual LRESULT LOOPER_SAFE postDelayedRunnable(shared_ptr<Runnable> obj, UINT ms);
	virtual void    LOOPER_SAFE cancelRunnable(shared_ptr<Runnable> obj);

```
是在Handler所有Looper运行Runnable.Run()

sendRunnable是执行Runnable完才返回  
postRunnable是投递后马上返回，Looper会稍后及时执行  
postDelayedRunnable是投递延时执行,ms是延时毫秒数  
cancelRunnalbe是取消postDelayedRunnable投递  

# 内部实现
CoreLooper内部采用消息+定时器来调用Runnable
