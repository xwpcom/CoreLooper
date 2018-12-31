#pragma once
namespace Bear {
namespace Core {
namespace Media {

//#define _CONFIG_TEST_ICAM_H265	//采用蒋工h.265来模拟测试


class VideoSource;
//XiongWanPing 2016.03.30
//从设备获取video数据并缓存
//供rtsp和录像使用
class AV_EXPORT VideoCapture :public Handler
{
	SUPER(Handler)
public:
	VideoCapture();
	virtual ~VideoCapture();

	virtual int OpenStream();
	virtual int CloseStream();

	void SetVideoSize(CSize sz)
	{
		mVideoSize = sz;
	}
	int SetMode(bool autoMode, int fps = 25);
	virtual void OnRequestNewFrame(VideoSource *source);
	virtual void InputFrame(LPBYTE frame, int frameLen);

	sigslot::signal3<LPBYTE, int, DWORD>	SignalVideoFrame;
	sigslot::signal1<VideoCapture*>	SignalOnOpen;
	sigslot::signal1<VideoCapture*>	SignalOnClose;
protected:
	void OnCreate();
	void OnTimer(long timerId);
	void applyConfig();

	virtual int GetNextFrame();

	int			mFileIndex;
	ByteBuffer	mFileBox;

	bool		mAutoMode;	//为true表示自动取下一帧，可导致VideoSource丢帧
							//为false时，仅在收到OnRequestNewFrame时才取下一帧
	int			mFps;
	CSize		mVideoSize = CSize(0, 0);

	long mTimerGetNextFrame = 0;
};

}
}
}
