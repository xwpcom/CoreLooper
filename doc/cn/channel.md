# Channel通道

CoreLooper采用Channel来抽象表示数据收发通道，比如TCP,串口,p2p连接等。  
Channel是Handler子类  

- Channel主要接口
```cpp
	virtual int Connect(Bundle& info) = 0;//bundle中传送连接所需的信息，比如ip,port,p2p id等
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen) = 0;
	virtual int Receive(LPVOID buf, int bufLen) = 0;
```

- Channel主要事件

```cpp
	sigslot::signal4<Channel*, long, ByteBuffer *, Bundle*>	SignalOnConnect;
	sigslot::signal1<Channel*>								SignalOnReceive;
	sigslot::signal1<Channel*>								SignalOnSend;
	sigslot::signal1<Channel*>								SignalOnClose;

```

 注意:  
 Channel接口已能稳定工作，但可能还需要细调一下，比如接口命名之类的细节  

 