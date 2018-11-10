# 源码准备中，预计2019年元旦之前上传
# CoreLooper简介
- CoreLooper是一个基于c++11的通用组件框架,支持通用业务和网络通讯。
- 与一般的网络框架不同，在框架设计层面CoreLooper更侧重于通用业务。
- CoreLooper对标Android java层和Windows MFC中的业务层，提供完善的基础设施。 
- CoreLooper目前不直接支持UI,但可以和UI层交互。后续可考虑集成OpenGL/DirectX UI.

[详细介绍请看这里](readme_cn.detail.md)

## CoreLooper主要功能
- 组件通讯, 跨线程通讯,网络通讯,三者和谐统一
- 组件命名,布局,定位
- 事件管理
- 定时器

## CoreLooper主要特点
- 跨平台,支持Android, iOS, Linux, Windows(x86/x64),可方便的移植到其他平台
- 不依赖其他库, 仅基于c++11(只用到shared_ptr,weak_ptr,auto等少量c++11特性,后续可能会加上lambda)
- 框架默默填坑，对用户无坑(或者说少坑),比如说用户不需要自己加锁
- 异步
- 简洁
- 具有无限扩展性
  

# CoreLooper能做什么
- 开发TCP,UDP服务器
- 开发设备端
- 开发客户端
- 开发产品SDK
- 开发串口应用
- 开发ActiveX/OCX控件
- 开发模拟器
- 开发其他通用库 CoreLooper提供了众多基础功能，可在此基础上构建你自己的库
-  其他应用场景,只要支持c++11，就可以使用CoreLooper
# CoreLooper愿景
- 希望能被广泛使用，成为行业事实标准
- 带动C++开源发展
- 成为C++标准

# Demo
## 最简单的main()函数
``` cpp
class MainLooper :public MainLooper_
{
	SUPER(MainLooper)
	
	void OnCreate()
	{
		__super::OnCreate();

		AddChild(make_shared<Handler>());
		PostQuitMessage(2018);
	}
};

int main()
{
	return make_shared<MainLooper>()->StartRun();
}
```
作为一个代码洁癖患者，理想情况下,不管多么复杂的app,我们建议所有采用CoreLooper框架的app，其main()代码应该像上面这样只有一行,或者像下面这样有3行(不能再多了),从这一细节也可以体现出CoreLooper框架的简洁之处。
```cpp
int main()
{
	auto obj=make_shared<MainLooper>();
	obj->StartRun();
	return obj->GetQuitCode();
}
```
# CoreLooper授权
CoreLooper是开源软件，但不是免费的，它采用灵活的授权方案。

## 在中国内地使用corelooper的授权
- 按C++开发人员收年费，每人每年100元
- 对在校大学生和无收入的用户每人每年0元,相当于免费使用
- 自愿原则

## 在其他国家和地区使用CoreLooper的授权(待定)


## CoreLooper授权收益会用来做什么
CoreLooper远没有完善，还有很多计划在我们的todo列表中，所有这些工作都需要大量的人力，物力和财力来实现。


