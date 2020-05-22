目录说明
demo		演示代码
depends		第三方依赖代码库
doc			文档
include		头文件
lib			生成的lib库(目前没用到)
projects	vs,android stuido,xcode工程
src			核心代码

.在linux下编译
	git clone后c和build.sh目前没有自带+x权限，正在研究自动加权限的办法，目前需要手工加上
	chmod +x ./c
	chmod +x ./build.sh
	运行./c
	或./build.sh即可
	编译后可运行demo,比如:
	cd corelooper/build/release/bin
	./timer
	
	显示编译详情
	./build.sh VERBOSE=1
	
	编译debug版
	BUILD_TYPE=Debug ./build.sh VERBOSE=1
	参考 https://blog.csdn.net/liuweihui521/article/details/52556375
	单步调试:
	cd CoreLooper/build/Debug/bin
	gdb -tui timer
	b main在main函数设置断点
	s是跟踪进函数
	n是执行下一条语句
	gdb调试太麻烦，还是vs方便些

.在windows下编译
	用vs2017打开projects\libcorelooper\corelooper.sln
	用vs2017打开demo\CoreLooperDemo.sln
	更多demo是通过unit test来做的
	直接编译,支持x86和x64

.在android studio下编译
	用android studio打开projects中以.as结尾的文件夹直接编译
	比如
	projects/libcorelooper.as
