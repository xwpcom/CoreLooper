# 在Windows下使用CoreLooper



## windows下获取代码时的注意事项

Visual Studio对字符集的支持不是太好，特别是代码里面有中文注释并且以\n换行时，vs有时会把部分注释当代码来编译，报一些莫名其妙的错误。

所以使用TortoiseGit在windows下获取代码时要注意CRLF转换

在右键菜单中>TortoiseGit>Settings>Git>Edit systemwide gitconfig中确保core.autocrlf=true

类似下面这样:

[core]
	autocrlf = true



## 使用visual stuido编译CoreLooper

使用visual stuido打开projects\libcorelooper\corelooper.sln即可编译

支持x86和x64



## 单元测试

在vs主菜单>Test>Windows>Test Explorer中打开单元测试面板

编译成功之后会在Test Explorer中列出所有的单元测试项目。

可以运行所有单元测试项目，或者只运行指定的项目。



## Demo

demo是通过unit test来做的

