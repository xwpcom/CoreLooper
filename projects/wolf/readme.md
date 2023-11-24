## build wolfssl for t21

用github上wolfssl 3.15.3现在能和hwCloud tls通讯
/mnt/corelooper/CoreLooper.git/projects/wolf/bin.t21
cmake -DCMAKE_TOOLCHAIN_FILE=./t21.cmake ..
make



## build wolfssl for ubuntu

/mnt/corelooper/CoreLooper.git/projects/wolf/bin.ubuntu
cmake  ..
make



## wolfssl for vs2023

在vs2023中打开wolfssl.sln，能编译运行,单步调试client 



