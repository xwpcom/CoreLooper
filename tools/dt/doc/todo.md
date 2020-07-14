
# 2020.07.14
- 采用比较笨的办法回避了LogPage中list.GetNextItem(-1, LVNI_SELECTED);异常问题


# 2020.07.07
- bug
LogPage::OnTimer中有不必要的刷新，原因是添加新item时list.GetNextItem(-1, LVNI_SELECTED);返回数值有变化

原因待查


# 2020.07.05
.强制要求text全部采用utf8编码，方便在list中正确解码显示中文


# 2020.06.19
todo:

- 下面的详情页面改用pane,方便自行调整窗口尺寸

# 2020.02.09
- 增加了限制最大条数mMaxLogCount

# 2020.02.08
changes:
- 实现了log list ctrl右键菜单的功能:open file and goto line,open folder,copy full path
- app,tag filter完成
- 完善了ItemPage
- 增加了level combo

todo:
- 实现Ctrl+Tab切换
- 用共享内存来优化，让日志尽量少占用app的时间

构想如下:
- app写日志到共享内存
- 做个LogServer,和app运行在同一主机,LogServer负责采集日志,用websocket提供日志服务
- client用websocket获取日志
- 以内存换取app性能

目前不需要这么高的性能，后面有需要了再说

# 2020.02.07

changes:
- 增加了open file goto line 功能
  
goto line是采用c#工程CoreLooper\tools\OpenFileGotoLine来做的

<pre>
https://stackoverflow.com/questions/350323/open-a-file-in-visual-studio-at-a-specific-line-number
https://docs.microsoft.com/en-us/visualstudio/ide/reference/devenv-command-line-switches?view=vs-2019
https://docs.microsoft.com/en-us/visualstudio/ide/reference/visual-studio-commands?view=vs-2019
https://docs.microsoft.com/en-us/visualstudio/ide/reference/go-to-command?view=vs-2019


devenv "Path\To\Your\Solution" /edit "Path\To\Your\File" /command "Edit.GoTo 123"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"  /edit "d:\t.cpp" /Command "Edit.GoTo 12"
只能打开文件，不能定位到行

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\dt.sln"  /edit "D:\CoreLooper\projects\dt\src\Consolepage.cpp" /Command "Edit.GoTo 12"
以文本方式打开了dt.sln并能定位到行

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"  /edit  /Command "Edit.GoTo 12"


"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"    /Command "Edit.GoTo 12"
在新的vs中能定位到行

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"  /edit "D:\CoreLooper\projects\dt\src\Consolepage.cpp" /Command "Edit.GoTo 12"

</pre>

# 2020.02.05
filter规则
- 多个条件之间采用逗号分隔

比如app=BearStudio,IotPlatform

为了实现方便和提高性能,字段区分大小写

- 支持! 比如app中有!PlatformServer时，屏蔽PlatformServer的日志





# 2020.02.04
发现目前的DT没法满足需要了,主要是缺少如下功能

- 分离多个进程的日志
   同时运行多个进程都向DT输出消息时，有时只需要关注某个进程

决定升级DT工具,要支持如下功能点

- 兼容旧的dt
- 增加tab页面，每个进程的日志可显示在各自的tab上面，便于调试多个app,互不影响

每条日志包含如下信息
- appName(exeName),pid,tid
- tag
- message
- log level:verbose,debug,warning,error...
- source file,line

## 功能设计
- 可根据各种参数过滤
- 优化WM_COPYDATA数据格式，采用TLV编码

## UI布局
有一个总tab
