# CoreLooperĿǰ֧��vs test��gtest
������ÿ������дһ��app��������֤�����鷳�����õ�Ԫ������ܷ��㣬����CoreLooper��Ҫ����unit test����֤����.

[Visual Studio��Ԫ����](../images/vs.unittest.png)

## ��λmemory leaks

��.cpp�м���

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

��Debug Selected Tests(ALT+X)���к�

��vs>Output>Debug���ӡmemory leaks��Ϣ����������

Detected memory leaks!
Dumping objects ->
d:\bear\share\bearcore\core\looper\tests\core_unittest.cpp(48) : {220} normal block at 0x05EB13E8, 4 bytes long.
 Data: <    > CD CD CD CD 
Object dump complete.



