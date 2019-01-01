# 智能指针
C++11引入的智能指针极大的方便了程序开发  
CoreLooper主要用到了shared_ptr和weak_ptr,再搭配make_shared,基本不再需要手工显式调用new和delete了  
为了演示这一点，特意实现了一个简化的ftp服务器，代码在  
demo\ftpserver


智能指针是目前我用过的最好的垃圾收回机制。  
它不仅把程序员从new,delete的困境中解放出来，搭配C++的RAII,它还能自动管理所有资源,这点是java gc做不到的。
另外，C++ smart pointer是确定性的。对象的生命周期其实还是由程序员的代码直接管理的，何时析构是非常明确的。而在java中对象构造是确定的，但其占用的资源在何时回收,程序员是无法确定的。




