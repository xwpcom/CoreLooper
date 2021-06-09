XiongWanPing 2018.12.22

这些代码要移植到c51,sim868和windows,linux上面
由于c51资源极小，所以要注意:
.不能调用alloc类函数和free
.不能占用过多stack空间
 c51 stack目前设置为默认在xdata空间，有3840字节

