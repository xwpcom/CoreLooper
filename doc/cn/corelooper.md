# CoreLooper简介

CoreLooper是一个基于c++11的通用组件框架,支持通用业务和网络通讯。

- 框架是什么  
简单来说,框架是一套可扩展，可重用的软件组件。一般是为了简化和规范某一方面的软件开发而设计。  
框架可分为通用框架和专用框架。  
通用框架不针对具体的业务，一般只实现最基本的功能，能适用于很多场合。  
专用框架则往往专注于某个细分领域，针对性很强。  
可以在通用框架的基础上构建专用框架。  
- 使用框架有什么好处  
用一句话就可以讲清楚:框架处理繁琐的通用细节问题，让用户集中精力处理应用层业务。  

## CoreLooper主要特点
开源界已经有众多网络框架,比如ace,asio,evpp,libevent,libuv,libev,muduo,但CoreLooper有与众不同的地方
- 提供通用完备的基础设施，不仅仅局限于网络应用
- 不依赖其他库, 仅基于c++11(只用到shared_ptr,weak_ptr,auto等少量c++11特性)
- 支持Android, iOS, Linux, Windows(x86/x64),可移植到其他平台
- 具有无限扩展性  
 比如可轻松实现所有设计模式,采用类java风格实现java开源库的功能，比如netty,eventbus,spring等
- 解决了多平台支持的难题  
  CoreLooper对所有平台都提供统一的接口，用户不需要关心每个平台的细节  
- 解决了thread粒度太大不好控制的问题  
   现在很少有软件是单线程的，为此各系统都提供了创建线程的接口，问题在于在c++层面，这个接口过于简陋。以pthread_create为例,它只需要提供一个线程入口函数ThreadFunc，看起来比较适合一条道跑到头。而在实际的业务中，线程经常需要和其他线程交互
- 解决了组件跨线程通讯难题  
 通过sendMessage和postMessage,多个线程中的组件可互相发送消息安全通讯 
- 解决了组件需要加锁才能保证线程安全的问题  
 在多线程应用中，不同线程中的组件经常需要互相调用，为了保证线程安全，经常需要加锁同步，所以在一般的app源码中可以看到大量的加锁调用。
 这其实是一个很严重的问题，在业务复杂时，一个锁可能完全不够用，而多个锁很容易造成死锁，并且调试排查比较困难，一旦出问题又是致命的。
 CoreLooper完全避免了这一窘境，各组件不需要自行加锁，一个锁也不要，就能实现跨线程安全通讯。
- 对象生命周期全自动管理，避免了手工分配和释放内存  
 这个其实是c++11智能指针做的，之所以在这里强调，是因为目前绝大多数开源软件并没有用到智能指针，仍然是采用手工管理，这会带来很多问题。CoreLooper框架大规模应用了智能指针，经验证发现效果很好，所以在此强烈推荐
- 可视化编程，改善了调试排查app的体验和效率  
这里说的可视化，不指是像在VS,AndroidStudio中那样开发 UI界面,而是能实时查看程序的运行状态数据。
在开发过程中，经常需要打日志来检查变量数据,打印多了会干扰其他功能的调节，打印少了有时想看的变量没打印出来，要修改代码，重新编译再测试，有时还不方便重现。
为了解决这个问题,BaseLooper在内置的HTTP服务器上增加了一个可扩展的ajax框架。可以用此框架来查看和修改数据，还特意增加了一个proc.xml接口，可以通过浏览器来查看组件的实时内部状态,能帮助开发人员快速定位bug。
## CoreLooper特别关注了现有知名网络开源库的一些痛点

- muduo

- evpp
 evpp readme中有一段话让我印象深刻,见: https://github.com/Qihoo360/evpp/blob/master/readme_cn.md
  摘抄如下:
> 我们实现过程中极其重视线程安全问题，一个事件相关的资源必须在其所属的`EventLoop`（每个`EventLoop`绑定一个线程）中初始化和析构释放，这样我们能最大限度的减少出错的可能。为了达到这个目标我们重载`event_add`和`event_del`等函数，每一次调用`event_add`，就在对应的线程私有数据中记录该对象，在调用`event_del`时，检查之前该线程私有数据中是否拥有该对象，然后在整个程序退出前，再完整的检查所有线程的私有数据，看看是否仍然有对象没有析构释放。具体实现稍有区别，详细代码实现可以参考 [https://github.com/Qihoo360/evpp/blob/master/evpp/inner_pre.cc#L36~L87](https://github.com/Qihoo360/evpp/blob/master/evpp/inner_pre.cc#L36~L87)。我们如此苛刻的追求线程安全，只是为了让一个程序能**安静的平稳的退出或Reload**，因为我们深刻的理解“编写永远运行的系统，和编写运行一段时间后平静关闭的系统是两码事”，后者要困难的多得多。

在CoreLooper框架中采用节点树来非常直观自然的实现了安全退出，evpp中说的很困难的事在CoreLooper中是个基本功能，并且很容易实现。  
[CoreLooper节点树](nodetree.md)



##  CoreLooper可以做什么

- 开发服务器
CoreLooper可用来开发高性能服务器，它支持TCP,UDP等网络协议,并且用户可自行挂靠其他信号机制到框架。
我们深知定时器对服务器来说是必不可少的，所以CoreLooper增加了对高性能定时器的支持，它在创建，销毁和调度定时器的时间复杂度都是O(1),能用于海量客户端连接的业务处理。

- 开发安防等行业设备端
CoreLooper已被移植到海思IPC方案,君正IPC方案, MTK6572等设备端,支持intel x86/x64,arm32/64,mips等芯片平台。

- 开发客户端
CoreLooper可用来开发Android, ios, linux和Windows等客户端
界面采用系统自带UI框架,涉及网络的业务可采用CoreLooper来做。
这样的好处是业务层只需要做一次，就能通用于所有系统平台,大大节省开发时间和成本。

- 开发产品SDK
在很多情况下，公司的软硬件产品都需要提供SDK开发给客户使用,这些SDK一般也要支持多个系统。这个听起来简单，实际做起来却会遇到很多问题。
举个简单的例子，要做一个SDK,此SDK要提供接口来通过网络连接上设备，然后做相关的操作。有一个前提是SDK要尽量做到让用户使用方便，而用户一般是在主UI线程中直接调用SDK接口，这就要求SDK接口都要做成非阻塞的。如何优雅的提供接口来实现这个功能，是有一点难度和工作量的。
CoreLooper最初就是基于这个需求而创建的,从一开始就考虑到这种应用，各方面都考虑了很多细节问题并完美解决了。
- 开发ActiveX/OCX控件
 尽管现代浏览器都不再支持OCX控件了，老的IE浏览仍然支持此项技术，CoreLooper开发
- 开发串口应用(比如RS232,RS485)
 CoreLooper已集成串口功能,实际上只要能用iocp,epoll和kevent关联的功能都可方便的集成到CoreLooper框架中
- 开发其他通用库
 CoreLooper提供了众多基础功能，可在此基础上构建你自己的库
- 其他应用场景
 只要支持c++11，就可以使用CoreLooper框架


## CoreLooper不能做什么
- CoreLooper可以和UI层配合使用，但目前CoreLooper不能直接做UI界面
 各操作系统的原生UI层已经做的足够好了。
- CoreLooper关注的是业务逻辑层，主要包括网络通讯，系统各组件的跨线程交互等。
- 理想组合是:系统原生UI层负责界面展示,CoreLooper负责业务和网络处理。
- 现在流行DirectUI,国人有开源类似的UI库, 感觉也是可以集成到CoreLooper中的,不需要进核心库,比如集成为libdui,像QQ,TIM的界面应该就是用DUI做的.
 值得一提的是github上有个opengl/DirectX做UI的项目imgui(https://github.com/ocornut/imgui),感觉也能集成到CoreLooper。用opengl/DirectX做UI可以跨平台，工作量也是扛扛的, UI这东西做到极致的话就会做成浏览器了。这是个大的话题，我想太多了，就此打住。


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

## CoreLooper设计理念

- KISS 保持简单易用
每个接口，包括但不限于命名,参数，返回值，语义等都经过精心考虑
- 宽容
框架要多多包涵app的无害用法，允许调用者犯错,但框架本身不能犯错
比如允许app多次调用同一Handler.Destroy()
- 无坑或少坑
框架尽最大努力提供各项服务，避免让用户踩坑
- 概念简单，使用方便
这点非常重要，使用框架的门槛要低，不要提太多概念，玩玄乎的名词概念,我们要的是实实在在的方便。
- 要跨平台，支持目前主流的操作系统
 各平台下要提供统一的接口,避免使用某平台独有的功能
- 代码要运行测试方便，便于快速试错迭代升级 
一个产品，不管是软件还是硬件，初期有bug,有缺陷并不可怕，这反倒是很正常的事情。只要能快速试错并升级，就能很快完善。对软件而言，这主要是尽量减少试错的时间成本，一个很有效的办法就是单元测试。


### app不需要自己加锁
框架采用消息队列，sendMessage/postMessage能解决几乎所有的问题,
app在使用时一般不需要自行同步
如果app用到了加锁，请重新考虑一下,确保明确知道自己在做什么。


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
- sendMessage的实现灵感来自google开源的libjingle  
  自从多年前接触windows编程以来，一直好奇SendMessage是怎么在等待回复时继续响应其他线程发来的窗口消息，找过reactos,minigui等开源库未果，最终在libjingle发现最简单的实现办法
- sigslot开源库  
 这个事件库很好用，CoreLooper只用到了它的单线程模式
  
- timewheel时间轮  
开源时间轮代码很多，CoreLooper采用的是最简洁的实现  [https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/](https://github.com/ape2010/ape_cpp_server/tree/master/frame/common/) 

