# CoreLooper Timer介绍
CoreLooper采用时间轮实现高性能定时器,创建,销毁和调用timer都基本上是O(1)复杂度。

每个Looper都内置timer管理，不需要单独的timer线程
## Timer接口
CoreLooper.Handler采用如下3个接口实现timer
```cpp
	virtual long SetTimer(long& timerId, UINT interval);
	virtual void KillTimer(long& timerId);
	virtual void OnTimer(long timerId);
```
设计timer时主要考虑可能有海量的Handler需要timer,但每个Handler只用到少量的timer

timer采用long标识，规定0是无效的timerId

interval单位:毫秒,至少为1,传0时内部自动转为1

SetTimer时如果传入timerId有效，则先删除此timer,然后再创建新的timer

返回时timerId为新创建的timer id,SetTimer返回值和timerId相同

KillTimer删除指定的timer id,返回时timerId被清0

成功删除后timerId会清0,失败时timerId保持原值不变

值得注意的是Handler.postDelayedRunnable在内部也是采用timer来实现的,其原型如下:
```cpp
virtual LRESULT LOOPER_SAFE postDelayedRunnable(std::shared_ptr<Runnable> obj, UINT ms);
```
### Timer注意事项
- 同一Looper中timer是依次串行执行的，所以在OnTimer中应该快速处理并及时返回，否则会导致其他timer延迟。  
timer理论精度1毫秒，实际取决于负荷。  

- CoreLooper保证在KillTimer(timerId)之后不会再触发此timerId的OnTimer
- 可以在OnTimer中随意调用SetTimer,KillTimer,全部会按期望的行为工作
- CoreLooper每次处理timer时为非重入，如果在OnTimer中调用sendMessage,在等待sendMessage返回的过程中不会再次触发looper的timer流程.这点应该问题不大，因为CoreLooper本来就建议快速处理,尽量全异步。如果一个业务可能占用较长时间，应该放在单独的looper中来做。

### Timer用法示例
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
		//对于本Handler自行创建的timer,不需要转给__super,处理后要及时return
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
		
		//运行到此时，timerId是基类创建的timer,要转给__super继续处理
		//这样能保证不管继承了多少级，每级Handler都可以响应并且只需要响应自已创建的timer
		__super::OnTimer(timerId);
	}
};
```
最佳用法:
- 每个timerId都采用类成员变量保存
- 在OnTimer中依照触发频率从高到低判断timerId,处理后及时return
  每层Handler应该只处理自己创建的timer,比如这里的DemoHandler.mTimerKeepAlive对基类来说是没有意义的，所以不需要调用__super::OnTimer来继续处理,直接return即可。

## 与Android Timer比较
如果没有看Android文档并实测验证，很难想象Android中每个Timer对象都采用单独的thread来做.

以下是android文档中关于Timer的描述:

Corresponding to each Timer object is a single background thread that is used to execute all of the timer's tasks, sequentially.

实测发现确实如此,这个行为是有点出乎预料的，目前还没弄清楚为什么这样设计

个人感觉Android.Handler.postDelayed和sendMessageDelayed更符合对timer的期望用法。

## 与Windows Timer比较
Windows中SetTimer原型如下
```cpp
UINT_PTR SetTimer( HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc );
```
它存在一个问题,nIDEvent即timerId是由app分配，MFC封装时也保持这样的用法，这样子类SetTimer时可能不知道基类已使用了哪些timerId,特别是在基类源码不可用的情况下。如果子类恰好选用了和基类相同的timerId,则app行为不太好预料。

为避免此问题，CoreLooper的解决办法是timerId由框架来自动分配,这样子类和基类不用担心timerId冲突。

另外，测试发现MFC app最多能创建10000个有效的timer,CoreLooper没有这个限制。

## 时间轮参考
网上开源的时间轮代码比较多，包括linux kernel中的也是采用类似方法，但个人感觉最简洁的是如下实现:
https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/  

CoreLooper timer的时间轮源码主要来源于这个项目，在此表示感谢!



