# Channelͨ��

CoreLooper����Channel�������ʾ�����շ�ͨ��������TCP,����,p2p���ӵȡ�  
Channel��Handler����  

- Channel��Ҫ�ӿ�
```cpp
	virtual int Connect(Bundle& info) = 0;//bundle�д��������������Ϣ������ip,port,p2p id��
	virtual void Close();

	virtual int Send(LPVOID data, int dataLen) = 0;
	virtual int Receive(LPVOID buf, int bufLen) = 0;
```

- Channel��Ҫ�¼�

```cpp
	sigslot::signal4<Channel*, long, ByteBuffer *, Bundle*>	SignalOnConnect;
	sigslot::signal1<Channel*>								SignalOnReceive;
	sigslot::signal1<Channel*>								SignalOnSend;
	sigslot::signal1<Channel*>								SignalOnClose;

```

 ע��:  
 Channel�ӿ������ȶ������������ܻ���Ҫϸ��һ�£�����ӿ�����֮���ϸ��  

 