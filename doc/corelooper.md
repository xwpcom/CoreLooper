
# CoreLooper框架概述
CoreLooper是一个基于C++11的全平台通用基础框架,灵感主要来自Android和Windows。

个人认为Android应用层的最值得称赞的是Handler和Looper这两个概念,而Windows主要是Message或者叫Event。这几个简单的概念接合C++11便可以构造出功能强大的通用框架，CoreLooper由此诞生。

由于Android中的组件名字取的太好了，我想不出其他更贴切的名称，所以基本沿用了Android中的名称。好处是望文生义，甚至可以直接看Android文档来了解CoreLooper中对应组件的用法，但某些地方也是有差别的，差别会在CoreLooper文档中详细说明，一般来说CoreLooper中的用法更加灵活一点.

在参考Android和Windows设计的同时,并没有完全固化照搬，CoreLooper做了一些融合和变通,形成自己的特色
- 采用C++11智能指针实现组件生命周期管理
- Handler采用统一接口实现同线程和跨线程通讯
- Looper是Handler子类
- 采用树形结构来分层管理Handler  
 内置Http Server和ajax框架可以在浏览器是方便的展示结构树信息  
 生命周期也体现在此结构树中，保证子节点全部析构了才可以析构父节点,MainLooper始终最后析构  
 以非常简洁的方式解决了360开源项目evpp中说的:  
 我们深刻的理解“编写永远运行的系统，和编写运行一段时间后平静关闭的系统是两码事”，后者要困难的多得多
- 采用sigslot进行事件管理
- 采用时间轮实现定时器
- 采用iocp/epoll/kevent实现网络功能
- 提供完善的机制能和Windows,Android,ios,Linux等平台的原生或第三方框架协同工作

上述这些特点以非常自然协调的方式共同实现了CoreLooper的功能，保证了框架能高效的处理所有业务。

CoreLooper作为一个简洁的基础框架，具有强大的可扩展性,可用在所有支持C++11的领域,包括但不限于:

 - 服务器开发
 - 嵌入式设备端开发
 - 客户端开发
 - SDK开发
 - 串口应用
 - 游戏,音视频
 - 模拟器
 - 所有能支持C++11的场景
 
可以把 CoreLooper当作一个基石,在此基础上架建所有的应用  
比如可以实现所有java开源框架的功能，像Netty,spring之类  
现有知名c/c++开源库,比如live555,如果采用CoreLooper来重构，感觉会简洁很多  
   
 目前CoreLooper专注业务和网络，没有直接包括UI开发，但可以和OS UI线程对接协作。  
 从理论上来说，在CoreLooper基础上用OpenGL/DirectX来开发类DirectUI功能是可行的,只是目前还没有时间和精力来做。
 
# 致谢
CoreLooper主要参考了Android和Windows的思路来设计,同时也深受一些开源思想的影响，特此答谢。

- 陈硕的"当析构函数遇到多线程── C++ 中线程安全的对象回调"  
 2016年看到这篇文章才开始使用智能指针并开始编写CoreLooper的前身BaseLooper,前段时间才知道这是其大作"Linux多线程服务端编程"的第一章。  
 现在可以说"怎么讲的都是常识",他那篇文章的目的达到了!
  
- sigslot开源库  
 这个事件库很好用，CoreLooper只用到了它的单线程模式
  
- timewheel时间轮  
开源时间轮代码很多，CoreLooper采用的是最简洁的实现  [https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/](https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/) 
 
# 互助

一个好的开源项目只凭热情是没有办法长久持续的。  
下面戏言一下(模仿马丁路德金和陈硕):  
我有一个梦想，CoreLooper被广泛使用，然后用户说"Bear,请收下我的膝盖和捐助",那我开源的目的就达到了。

