目录说明
demo		演示代码
depends		第三方依赖代码库
doc			文档
include		头文件
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

.在windows下编译
	用vs2017打开projects\libcorelooper\corelooper.sln
	用vs2017打开demo\CoreLooperDemo.sln
	更多demo是通过unit test来做的
	直接编译,支持x86和x64

.在android studio下编译
	用android studio打开projects中以.as结尾的文件夹直接编译
	比如
	projects/libcorelooper.as
