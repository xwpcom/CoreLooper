# 在linux下使用CoreLooper
​	git clone后c和build.sh目前没有自带+x权限，正在研究自动加权限的办法，目前需要手工加上
​	chmod +x ./c
​	chmod +x ./build.sh
​	运行./c
​	或./build.sh即可
​	编译后可运行demo,比如:
​	cd corelooper/build/release/bin
​	./timer
​	

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