#pragma once
namespace Bear {
namespace Core {
namespace Media {
//XiongWanPing 2009.09.28简化
class RingBuffer
{
public:
	RingBuffer();
	virtual ~RingBuffer();

	int MaxReadSize();
	int MaxWriteSize();
	int GetBufferSize();

	int Init(int ulSize);
	VOID Destroy();
	void Clear()
	{
		mReadPos = mWritePos;
	}

	int Write(LPBYTE pData, int len, int &writeBytes);
	int Read(LPBYTE pData, int len, int& readBytes);

protected:

	int		mTotalSize;			/*Buffer 的总长度*/
	LPBYTE	mRB;				/*Buffer的开始地址*/
	int		mWritePos;		/*写的位置, 偏移位*/
	int		mReadPos;		/*读的位置, 偏移位*/
};
}
}
}