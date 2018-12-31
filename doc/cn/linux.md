# CoreLooper在Linux下的使用

## signal
我对signal没什么好感  
CoreLooper中采用LinuxSignal对几个signal做了屏蔽，然后就没再理会signal了  

Signal在某些场合可能有点用，但它带来的麻烦比好处多，感觉应该有更好的办法来实现同样的效果  
一个简单的例子是SIGPIPE默认中止app,即便可能是历史原因，我也觉得这样是不好的。

在技术选择方面，我更喜欢全平台通用的，这样更具有普适价值一些。  
Windows上win32 gui框架没有signal机制也能完全满足所有场景的需求,说明signal不是必须的,就是这样。  
