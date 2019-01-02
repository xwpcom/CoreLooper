# LOOPER_SAFE,SUPER,\_Object,\_MObject,\_Shortcut等几个常用宏的说明
## LOOPER_SAFE
表示跨Looper安全

## SUPER
微软vc++有个扩展关键字__super非常好用，但gcc没有提供，所以CoreLooper采用SUPER来模拟，  
```
//然后就能在代码中使用__super来引用基类了  
```
## _Object
用来在当前Looper中查找Handler,是类型安全的
可以看到这么一长串是比较麻烦的，所以采用宏来简化一下
```cpp
//FindObject in current looper
#define _Object(className,url) dynamic_pointer_cast<className>(Looper::Object(url))
```

## _MObject
在MainLooper中查找Handler,是LOOPER_SAFE跨Looper安全和类型安全的
```cpp
//FindObject in main looper
#define _MObject(className,url)  dynamic_pointer_cast<className>(Looper::GetMainLooper()?Looper::GetMainLooper()->FindObject(url):nullptr)
```

## _Shortcut
快捷方式，感觉其实用_S更快捷,忍了吧!
```cpp
#define _Shortcut(className,name) dynamic_pointer_cast<className>(Shortcut(name))
```

