# ����CoreLooper����SDK
���Ի���CoreLooper���������Ʒ������ƽ̨��SDK
����SDK���Ĳ���C++�������ٽ�Ͼ����OS���װһ�����㡣

��ͬOS���ں˺��ϲ�api�ӿ�,����Ʋ�ƷSDKʱҲҪ��ͬ����˼·������SDK��Ϊ�ں˺��ϲ�ӿڡ�
�ں���SDK�ڲ���С����(���Զ���ʥ��ʿ��ʸ),����SDK�ڲ���MainLooper,�����ṩһЩ�ӿ�����MainLooper������

## SDK��һ����ԭ��
- �ԼҲ�Ʒ����ͬ����SDK�������������ṩ���ͻ���SDK��ͬһ��
 �����ɱ����˽������ʡʱ�䣬Ҳ�������֪�ͻ���ʹ��SDKʱ���������Ⲣ��һʱ����
- ���ּ��
- �첽��������
 Ϊ����ʹ�ã�һ�㾡�����û������̵߳���SDK�ӿڣ�Ϊ�˲�����UI���棬�ӿڶ�Ӧ���Ƿ�������  
 ���ĳ�����ܲ��ܼ�ʱ��ɣ����Բ��Ϊ�������裬������ɺ�ͨ���¼���֪ͨ  
 ����Զ�������豸�����������޷���֤��ʱ���ӳɹ������Բ��ΪStartConnect��OnConnectAck��
- Ҫ֧��Unit Test  
 ��Ԫ���Ե���Ҫ����ôǿ����������
- �ṩ����ʱ��״̬��Ǩͼ��
 �û������һ���Ϥ�Ĳ�Ʒ��SDKʱ������һ��ͷ����û�У�������ṩͼ�Ĳ�ï���ĵ����û��ܺܿ����֡�

## Android��SDK
AndroidӦ�ò�һ�����java������������ͨ��NDK����jni�ӿ�����SDK,��������:  
```java
class JavaClass
{
	private JavaClassJni mJni;
}

class JavaClassJni
{
	//�ڲ�ͨ��jni����CppClass�ӿ�
}
```
```cpp
class CppClass
{
};
```
��androidӦ�ò㣬ֻ��Ҫֱ�ӵ���JavaClass�Ľӿ�,����Ҫ����JavaClassJni  
JavaClass�ڲ�����JavaClassJni  
JavaClassJniͨ��jni����CppClass

������java������Ա��˵���ӿ�����Ϥ��java�࣬���ýӴ�İ����C++�ӿڡ�

todo:
- �ṩdemo����ʾ�÷�

## IOS��SDK
todo

## Linux��SDK
todo

## Windows��SDK
todo


