# MainLooper
人无头不走，鸟无头不飞。  
凡事都讲究有个源头
CoreLooper中引入了MainLooper的概念
作用类似Android中的Application对象和MFC中的CWinApp。
一个app中应该有且仅有一个MainLooper

MainLooper只是一个占位符，普通Looper调用SetMainLooper(this)后就是MainLooper了,用户可使用其他名字
MainLooper相关接口如下
```cpp
	static Looper *GetMainLooper();
	static int SetMainLooper(Looper*);
	static bool IsMainLooper(LooperImpl *looper);
```

为方便使用，CoreLooper定义了帮助类MainLooper_
```cpp
//main looper helper
class CORE_EXPORT MainLooper_ :public Looper
{
public:
	MainLooper_()
	{
		auto name = "MainLooper";
		SetObjectName(name);
		mThreadName = name;
		SetMainLooper(this);
	}
};
```
这样一来，可简化为
```cpp
	class MainLooper :public MainLooper_
	{
		//...
	};
```

MainLooper主要是用在main函数中,比如

```cpp
int main()
{
	return make_shared<MainLooper>()->StartRun();
}
```
