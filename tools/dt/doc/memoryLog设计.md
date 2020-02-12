XiongWanPing 2020.02.10
# Memory Log优点
- 可尽量减轻log对app的性能影响
- log始终是同步的
- 
# Memory Log缺点
- 占用内存多一点
- 当缓存较小并且日志较频繁导致LogServer来不及取出时，可能丢失较旧的日志
- 需要单独的进程来取出日志

如果分配的内存足够大，则不会丢失日志

# Memory Log设计
- header+TLV log items
- 由于内存有限，要支持检测log丢失

# Memory Log构架
## 组件
- app,是需要打印log的程序
- LogServer,负责采集app的log,并对外提供日志服务

## 工作简介
app创建共享内存，打印log
LogServer和app运行在同一host,从app创建的共享内存中提取log,并对外提供日志服务,比如通过websocket传给浏览器展示

## LogServer如何检测app和shared memory log
有如下几种可行的方式
- 由用户手工登记app信息到LogServer,app采用app.+pid命名sm,LogServer定时枚举process
- 由LogServer创建shared memory,app采用mutex同步统一打印到此sm

# 参考资料
- g3log
- https://github.com/gabime/spdlog
- 