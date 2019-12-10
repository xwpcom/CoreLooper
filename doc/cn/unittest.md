# CoreLooper目前支持vs test和gtest
单独给每个功能写一个app来测试验证过于麻烦，而用单元测试则很方便，所以CoreLooper主要采用unit test来验证功能.

[Visual Studio单元测试](../images/vs.unittest.png)

## 定位memory leaks

在.cpp中加上

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

用Debug Selected Tests(ALT+X)运行后

在vs>Output>Debug会打印memory leaks信息，样本如下

Detected memory leaks!
Dumping objects ->
d:\bear\share\bearcore\core\looper\tests\core_unittest.cpp(48) : {220} normal block at 0x05EB13E8, 4 bytes long.
 Data: <    > CD CD CD CD 
Object dump complete.



