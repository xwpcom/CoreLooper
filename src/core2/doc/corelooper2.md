XiongWanPing 2022.12.18



# CoreLooper2计划

在corelooper使用过程中发现设计上有好的地方，也有需要改进的地方

公司现有项目已经大量使用corelooper框架，没法直接大幅修改，只能重新构建corelooper2，在上面应用新的想法,等成熟后再在新项目使用

## 类型标识改进

不再使用windows类型标识，比如BYTE,WORD,DWORD等，而是采用c++11标准类型标识,比如uint8_t,uint16_t这样的

即明确指出类型的长度

## Log日志改进

~~可参考spdlog~~,spdlog过于复杂

采用stream来做日志

```c++
LogV(TAG,"%s","hello");
```

改用

```c++
LogV(TAG)<<xx<<yy;
```

log要支持循环保存到文件



很多日志系统都需要初始化才能正常工作，这样有时不太方便，比如初始化全局变量时没法打印日志

core2采用可扩展的方式来支持Log

可支持directLog和initedLog

directLog是指不需要初始化，直接就能打印日志

initedLog是指需要初始化后才能使用日志



## Handler

###  proc从xml改为json

之前用xml是因为chrome原生就能展示xml数据，需要装插件才能支持.json展示

现在看来json应用更方便一些

vue,微信小程序，开源json使用都比xml简单



### BindProcData已过时

它的缺点是不够灵活

改进措施:改用json,由Handler在响应时明确返回数据



## ObjectTree

明确约定对象树解析定位方式

用[]支持映射child

```c++
shared_ptr<Handler> mapChild(const string& token);
    
/IotServer[Uid][12]
/IotServer[Uid][12].xx
    
/,[]和.的用法需要仔细规划一下
    
```





## 文件全部采用utf8 with bom保存

之前一些嵌入式设备的编译器，不支持utf8 bom

目前用的嵌入式设备编译器都能支持bom了



## 编译 0 warning

我们自己的代码编译时不能出现任何警告

引入的第三方代码不作此要求





## 支持udp

## 支持http和https

## namespace避免过多层次

一般采用两级即可,比如

Bear::Core2

Bear::Net

Bear::Http

Bear::Ftp

不要搞成Bear::Net::Http这样的



## Net要支持多个looper并行

## 引入wepoll，在windows下不再自行使用iocp

wepoll使用了microsoft没公开的接口，可能会有风险

但java官方都用了wepoll,说明其可靠性是有保障的

好处是可以和linux epoll,macos kqueue的用法统一



## 可以但尽量少用template

## 每个模块都内置profiler功能

便于检查性能瓶颈

