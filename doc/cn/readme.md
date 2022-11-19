# CoreLooper简介
- CoreLooper是一个基于c++11的全平台通用组件框架,支持通用业务和网络通讯。
- 与一般的网络框架不同，在框架设计层面CoreLooper更侧重于通用业务。
- CoreLooper对标Android java层和Windows MFC中的业务层，提供完善的基础设施。 
- CoreLooper目前不直接支持UI,但可以和UI层交互。后续可考虑集成OpenGL/DirectX UI.

## 主要功能
- 组件通讯, 跨线程通讯,网络通讯,三者自然统一
- 组件命名,布局,定位
- 事件管理
- 定时器

## 主要特点
- 对象生命周期管理，对象同线程/跨线程通讯，事件等基础设施完善，让用户专注于自身业务
- 能采用android java style来写C++
- 采用智能指针管理c++对象,避免手工new,delete等琐碎任务,并且和框架良好匹配
- 框架帮助用户养成有始有终的编码习惯,app都必须干净的退出
- 不依赖其他库, 仅基于c++11(只用到shared_ptr,weak_ptr,auto等少量c++11特性,后续可能会加上lambda)
- 框架默默填坑，对用户无坑(或者说少坑),比如说用户不需要自己加锁
- 异步
- 简洁
- 具有无限扩展性
- 跨平台,支持Android, iOS, Linux, Windows(x86/x64),并可方便的移植到其他平台
  

## CoreLooper能做什么
- 开发TCP,UDP服务器
- 开发设备端
- 开发客户端
- 开发产品SDK
- 开发串口应用
- 开发ActiveX/OCX控件
- 开发模拟器
- 开发其他通用库 CoreLooper提供了众多基础功能，可在此基础上构建你自己的库
-  其他应用场景,只要支持c++11，就可以使用CoreLooper
# 愿景
- 希望能被广泛使用，成为行业事实标准
- 带动C++开源发展
- 成为C++标准

为什么定这么"宏大"的目标呢? 主要是风闻asio要进c++标准，这让我大吃一惊。  
个人感觉asio把简单的问题复杂化了。  
尽管CoreLooper目前没有asio完善，但应该比asio简洁很多,易用性也有天壤之别。  
做人还是要有理想的，万一实现了呢!  



# [文档目录](index.md)

点击[文档目录](index.md)可以查看到详细的文档。



# Demo
## 最简单的main()函数
``` cpp
class MainLooper :public MainLooper_
{
	SUPER(MainLooper_)
	
	void OnCreate()
	{
		__super::OnCreate();
	}
};

int main()
{
	return make_shared<MainLooper>()->StartRun();
}
```
作为一个代码洁癖患者，理想情况下,不管多么复杂的app,我们建议所有采用CoreLooper框架的app，其main()代码应该像上面这样只有一行(不能再多了),从这一细节也可以体现出CoreLooper框架的简洁之处。

# CoreLooper授权
CoreLooper是免费开源软件

## [捐助](donate.md)
如果觉得CoreLooper对您有帮助，可以在自愿的基础上[捐助](donate.md)本项目

# CoreLooper为什么要开源

开放能带来更大的价值  
本人比较喜欢近现代西方科学和开源软件,近几百年科技发展迅猛，一个很重要的原因是开放的精神,心向往之。      

# 反馈问题

bug、修改建议、疑惑都欢迎提在issue中，或加入CoreLooper QQ群讨论问题。
## QQ群
QQ群:484544131

或扫码进群

![img](../images/qq.group.png)

作者  
Bear熊万平  2018.12.31

