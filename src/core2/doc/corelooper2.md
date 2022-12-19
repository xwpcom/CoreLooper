XiongWanPing 2022.12.18



# CoreLooper2计划

在corelooper使用过程中发现设计上有好的地方，也有需要改进的地方

公司现有项目已经大量使用corelooper框架，没法直接大幅修改，只能重新构建corelooper2，在上面应用新的想法,等成熟后再在新项目使用

## 类型标识改进

不再使用windows类型标识，比如BYTE,WORD,DWORD等，而是采用c++11标准类型标识,比如uint8_t,uint16_t这样的

即明确指出类型的长度

## Log日志改进

可参考spdlog

采用stream来做日志

```c++
LogV(TAG,"%s","hello");
```

改用

```c++
LogV<<TAG<<xx<<endl;	
```

但stream log有个问题，怎么支持TAG?

log要支持循环保存到文件





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









