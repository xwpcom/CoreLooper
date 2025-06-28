# 2025.06.15
增加了dt.path.json
qt在vs2022中编译时__FILE__是相对路径,为了能方便定位日志文件,需要将相对路径转换为绝对路径
可在dt.path.json中增加工程路径，然后dt会自动搜索匹配为绝对路径
这样双击就能在vs中定位到日志代码行了

https://forum.juce.com/t/clang-make-file-use-absolute-path/41874
When compiling with MSVC, the /FC flag can be used to make the __FILE__ macro resolve to the absolute path of the current source file.
I added -working-directory=$PROJECT_DIR to the Xcode compiler flags and now __FILE__ is working as expected, giving me the absolute path to the current source.


# 2021.09.22
LogPage::OnCopyAll()中在loop中调用T2A可导致stack overflow
//已改


# 2021.01.10
todo:
.在家里的电脑上面,权限没问题，但OpenFileGotoLine也是不能定位
 有待研究

# 2021.01.07

今天在测试ocx时,vs需要管理员权限才能成功注册ocx

发现此时DT接收不到管理员级别的消息，双击日志也不能在vs中定位到代码行

DT也采用管理员权限运行后，就能正常接收和定位到代码行了

后面需要研究一下权限问题



# 2020.12.23

changes:
.勾上AutoScroll后，当ListCtrl中有mouse,keyboard操作时，设AutoScroll为三态，并在AutoScroll旁边搞一个倒计时，超时才真正scroll

# 2020.12.16
LogV 10K条
ConsolePage::OnCopyDataReady中LogParser::Input约547 ms
发现CListCtrl virtual list的SetItemCountEx很慢，凡是可能频繁调用到SetItemCountEx的地方都需要优化

changes:
.增加了ListCtrl virtual以提高性能
.增加auto scroll checkbox,手工控制更方便

todo:
.tag部分匹配也要显示出来
.ribbon上做个combox,可选择常用的filter
 可add,edit,remove filter

# 2020.12.15
发现dt有时没响应，不是bug,而是filter改动后处理的太慢，急需virtual list改进
https://www.codeguru.com/cpp/controls/listview/advanced/article.php/c4151/MFC-Virtual-List-Control.htm

# 2020.11.26
- 采用CListCtrl virtual来加速
https://blog.csdn.net/business122/article/details/78336945?utm_source=blogxgwz3

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
