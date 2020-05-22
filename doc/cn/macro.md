# LOOPER_SAFE,SUPER,\_Object,\_MObject,\_Shortcut�ȼ������ú��˵��
## LOOPER_SAFE
��ʾ��Looper��ȫ

## SUPER
΢��vc++�и���չ�ؼ���__super�ǳ����ã���gccû���ṩ������CoreLooper����SUPER��ģ�⣬  
```
//Ȼ������ڴ�����ʹ��__super�����û�����  
```
## _Object
�����ڵ�ǰLooper�в���Handler,�����Ͱ�ȫ��
���Կ�����ôһ�����ǱȽ��鷳�ģ����Բ��ú�����һ��
```cpp
//FindObject in current looper
#define _Object(className,url) dynamic_pointer_cast<className>(Looper::Object(url))
```

## _MObject
��MainLooper�в���Handler,��LOOPER_SAFE��Looper��ȫ�����Ͱ�ȫ��
```cpp
//FindObject in main looper
#define _MObject(className,url)  dynamic_pointer_cast<className>(Looper::GetMainLooper()?Looper::GetMainLooper()->FindObject(url):nullptr)
```

## _Shortcut
��ݷ�ʽ���о���ʵ��_S�����,���˰�!
```cpp
#define _Shortcut(className,name) dynamic_pointer_cast<className>(Shortcut(name))
```

