#pragma once
namespace Bear {
namespace Core {
namespace Media {

//XiongWanPing 2016.03.31
//表示一帧视频数据
class AV_EXPORT VideoFrame :public ByteBuffer
{
public:
	VideoFrame();
	virtual ~VideoFrame();

	DWORD GetFrameIndex()const
	{
		return mFrameIndex;
	}

	void SetFrameIndex(DWORD frameIndex)
	{
		mFrameIndex = frameIndex;
	}

	ULONGLONG GetTimeStamp()const
	{
		return mTimeStamp;
	}
	void SetTimeStamp(ULONGLONG timestamp)
	{
		mTimeStamp = timestamp;
	}

	bool IsKeyFrame()const
	{
		return mKeyFrame;
	}

	void SetKeyFrame(bool keyFrame)
	{
		mKeyFrame = keyFrame;
	}

	shared_ptr<VideoFrame> Clone();

protected:
	DWORD		mFrameIndex;//用来唯一标识每个帧
	ULONGLONG 	mTimeStamp;
	bool		mKeyFrame;
};

}
}
}
