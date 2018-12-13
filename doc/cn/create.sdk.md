# 基于CoreLooper创建SDK
可以基于CoreLooper框架来给产品创建跨平台的SDK
建议SDK核心采用C++开发，再结合具体的OS层封装一个薄层。

如同OS分内核和上层api接口,在设计产品SDK时也要按同样的思路，即把SDK分为内核和上层接口。
内核是SDK内部的小宇宙(来自动漫圣斗士星矢),比如SDK内部用MainLooper,对外提供一些接口来和MainLooper交互。

## SDK的一般性原则
- 自家产品基于同样的SDK来开发，即和提供给客户的SDK是同一份
 这样可避免藏私货，节省时间，也能切身感知客户在使用SDK时遇到的问题并第一时间解决
- 保持简洁
- 异步，非阻塞
 为方便使用，一般尽量让用户在主线程调用SDK接口，为了不阻塞UI界面，接口都应该是非阻塞的  
 如果某个功能不能及时完成，可以拆分为几个步骤，比如完成后通过事件来通知  
 比如远程连接设备，网络连接无法保证及时连接成功，可以拆分为StartConnect和OnConnectAck。
- 要支持Unit Test  
 单元测试的重要性怎么强调都不过分
- 提供调用时序，状态变迁图等
 用户在面对一款不熟悉的产品和SDK时，可能一点头绪都没有，如果能提供图文并茂的文档则用户能很快上手。

## Android版SDK
Android应用层一般采用java来开发，可以通过NDK开发jni接口来做SDK,建议如下:  
```java
class JavaClass
{
	private JavaClassJni mJni;
}

class JavaClassJni
{
	//内部通过jni调用CppClass接口
}
```
```cpp
class CppClass
{
};
```
即android应用层，只需要直接调用JavaClass的接口,不需要调用JavaClassJni  
JavaClass内部调用JavaClassJni  
JavaClassJni通过jni调用CppClass

这样对java开发人员来说，接口是熟悉的java类，不用接触陌生的C++接口。

todo:
- 提供demo来演示用法

## IOS版SDK
todo

## Linux版SDK
todo

## Windows版SDK
todo


