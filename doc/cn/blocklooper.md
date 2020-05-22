# BlockLooper
理想情况下所有Looper都应该非阻塞，但理想很美好，现实很残酷，有时难免需要执行一些阻塞性操作，为了便于框架事务管理，提供了BlockLooper子类。  
建议所有可能执行阻塞性操作的Looper都应该为BlockLooper子类,这样很容易分辨出哪些Looper可能阻塞。

无论何时,MainLooper不应该阻塞。

为了方便调试，MainLooper一般内置HttpServer,通过ajax提供proc.xml  
这样可通过浏览器查看Handler结构树,所有关键信息都应该能及时的展示。  
意思就是主要阻塞主Looper,通过主Looper可以了解系统运行情况

- todo:
关于BlockLooper的状态信息怎么展示在proc.xml中，这个有待研究完善
 一个也许可行的办法是采用shadow buddy,即当检测到是block looper节点的数据时，在MainLooper创建代理 Handler来完成。或者重构ProcManager,找到其他更好的办法,细节有待研究。
 





