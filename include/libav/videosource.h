#pragma once
#include "av_inc.h"
#include "videoframe.h"

namespace Bear {
namespace Core {
namespace Media {

class VideoCapture;

struct tagH264Or5BoxInfo
{
	tagH264Or5BoxInfo()
	{
		mValid = false;
	}

	bool		mValid;
	ByteBuffer	mVPS;
	ByteBuffer	mSPS;
	ByteBuffer	mPPS;
};

enum
{
	BM_GET_H264_5_BOX_INFO = 1,//wparam is tagH264Or5BoxInfo*
	BM_GET_NEXT_FRAME,
};

//XiongWanPing 2016.03.30
//从VideoCapture设备获取video数据并缓存
//供rtsp和录像使用(从mFrames.ByteBuffer中复制，而不是直接引用)
class AV_EXPORT VideoSource :public Handler
{
	SUPER(Handler)
public:
	VideoSource();
	virtual ~VideoSource();

	//GetNextFrame可以跨looper调用
	virtual std::shared_ptr<VideoFrame> LOOPER_SAFE GetNextFrame(DWORD indexHasGet);

	void KeepAlive();

	std::shared_ptr<VideoCapture> GetCapture()
	{
		ASSERT(IsMyselfThread());
		return mCapture.lock();
	}

	ULONGLONG GetLastInputTick()const
	{
		return mLastInputTick;
	}
	ULONGLONG GetLastUserTick()const
	{
		return mLastUserTick;
	}

	void ClearCacheFrames()
	{
		ASSERT(IsMyselfThread());
		mFrames.clear();
	}

	bool IsSpecialFrameReady()const
	{
		switch (mVideoInfo.mPixelFormat)
		{
		case V4L2_PIX_FMT_H264:
			return mHasGetSpsPps;

		case V4L2_PIX_FMT_H265:
			return mHasGetSpsPps && mHasGetVPS;
		}

		return false;
	}

	int SetVideoMediaInfo(const tagVideoMediaInfo& info);
	int GetVideoMediaInfo(tagVideoMediaInfo& info);
	CSize GetVideoSize()
	{
		return CSize(mVideoInfo.mWidth, mVideoInfo.mHeight);
	}

	virtual void InputFrame(LPBYTE frame, int frameLen, DWORD flags = -1);

	virtual bool IsEOF()const
	{
		return mEOF || mClosed;
	}

	virtual void OnVideoStreamOpen(VideoCapture*);
	virtual void OnVideoStreamClose(VideoCapture*);

	string GetUri()const
	{
		ASSERT(IsMyselfThread());
		return mUri;
	}

	void SetUri(string uri)
	{
		ASSERT(IsMyselfThread());
		mUri = uri;
	}
	string GetTitle()const
	{
		ASSERT(IsMyselfThread());
		if (mTitle.empty())
		{
			return GetUri();
		}

		return mTitle;
	}

	void SetTitle(string name)
	{
		ASSERT(IsMyselfThread());
		mTitle = name;
	}

	void SetVod(bool vod);
	sigslot::signal1<VideoSource*>	SignalRequestNewFrame;
	std::weak_ptr<VideoCapture> mCapture;
protected:
	void OnCreate();
	//void OnTimer(long timerId);
	virtual void OnSpeicialParamsReady();

	int OnRecvVideoSampleHelper(LPBYTE pSample, UINT cbSample, DWORD flags);
	int GetKeyFrameCount();

	LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);

	bool isVPS(BYTE naluType);
	bool isSPS(BYTE nal_unit_type);
	bool isPPS(BYTE nal_unit_type);

protected:
	tagVideoMediaInfo	mVideoInfo;
	ByteBuffer			mVPS;//h.265专用
	ByteBuffer			mPPS;//已去掉00000001前缀
	ByteBuffer			mSPS;
	bool				mHasGetSpsPps;
	bool				mHasGetVPS;

	std::list<std::shared_ptr<VideoFrame>> mFrames;//优化时可改用一个ByteBuffer来循环缓存所有帧数据,需要记住所有的帧边界

	bool				mVod;
	bool				mEOF;
	bool				mClosed = true;
	DWORD				mCurrentFrameIndex;//用来唯一标识每个帧,25fps时经过49*40天才回绕,所以不必担心回绕问题
#ifdef _DEBUG
	int					mFrameIndex;
#endif

	string			mUri;//rtsp stream uri
	string			mTitle;//码流标签,可用来标识码流,在用户界面上可据本翻译为适当的名称,比如mTitle为MainStream时显示为主码流

	ULONGLONG			mLastInputTick;//记录最后一次接收到视频帧的时间，可用来检测采集是否正常
	ULONGLONG			mLastUserTick;//记录最后一次有客户端调用GetNextFrame的时间，用来做智能停流
	//当超过一定时间没有客户端取流时，板端可自动停流
	//当有新连接进入时，板端自动开流
	//可节省电量
	void RefreshUserTick()
	{
		mLastUserTick = ShellTool::GetTickCount64();
	}

	std::shared_ptr<VideoFrame> GetNextFrame_Impl(DWORD indexHasGet);
};

}
}
}
