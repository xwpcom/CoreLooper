# todo
CoreLooper已初步可用,还有如下功能有待完善
- 写English文档
- 网络接口重构
 目前还在处理Handler和Looper细节，后续才有时间整理网络类接口
- 完整UnitTest和各功能演示用法 
- 支持RPC
- HTTPS
 主要卡在非阻塞TLS握手,openssl和mbedtls都没有提供好用的接口
- WebSocket Server
- 通用的dns async parser
 现在是用一个DnsLooper内部以阻塞方式实现,有待改进
- Log日志
 需要满足如下条件  
 高性能，不影响主业务  
能条件编译去掉  
能根据条件过滤  
简洁

- Looper池
 在现有CoreLooper框架下实现一个Looper池应该简单，但还没想到好的应用场景
- 重构优化proc.xml
 主要是需要想个好办法以非阻塞的方式把BlockLooper分支里的数据也显示在proc中,可能考虑shadow buddy
- 统一处理消息体投递失败时的析构
 只出现在Looper已停止，再直接对Looper发消息的场合,此时send,post会失败，所以参数中的make_shared<struct>不能用mSelfRef指向自己来保活
 对普通Handler不会出现此问题
- TimerManager优化
 记住上次tick和版本号
 每次timer有变动时版本号+1
 当TimerManager::ProcessTimer中tick和version都和上次相同时，说明和上次调用完全相同，直接返回上次的结果
 这样可省下大量的重复检测
 版本号没有变化时，可直接用缓存tick减去当前tick,不需要每次都遍历时间轮
- 改进timer精度
 在windows下面有时会有细微漂移，Linux下很精确,原因还没时间查
- Handler在析构时post消息给looper
 这样looper在quit时就不需要settimer来定时检测是否可能quit looper了
 只有当Handler的parent为looper时才需要这样做
 如果handler的parent也是handler,则不需要post给looper
- 要考虑多个基于corelooper的库怎么布局
 可参考MFC 多个dll的处理
 MFC中每个extension dll都有一个app对象，然后.exe自己也有一个app,看它内部是怎么处理的
- 引入lambda和协程
 前提是要保持简洁好用，否则没有必要
- 感觉Handler要增加action chain的概念
 经常有这样的需求，一个业务的流程比较多，需要分多个步骤
 如果按常规做法，需要分散在多个地方转接处理,比如消息来转去,这样不是很清晰
 action chain的好处是集中在一处把流程制订清楚,执行时按步就班
可参考下面的项目来
https://github.com/xhawk18/promise-cpp
https://github.com/Amanieu/asyncplusplus/wiki/Parallel-algorithms
- 关于帮助app测试跨looper析构Handler安全性
 可增加一个bit,试图析构时转交给另一个Looper,如果没能成功析构，再返回给原looper
- 绝大部分Handler可能不需要name和shortcut,所以tagHandlerInternalData可优化一下，做一个shared_ptr<>结构，仅在需要时创建
 这样每个tagHandlerInternalData可节省几十个字节

- 改进DT工具
一直以来想把DT.exe由现在的ListCtrl改为RichEdit,像Android Studio中的logcat那样  
主要是想支持过滤  
- 整理串口类
- ...
