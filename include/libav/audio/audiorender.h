#pragma once
#include "ringbuffer.h"

namespace Bear {
namespace Core {
namespace Media {

//XiongWanPing 2012.05.15
//负责播放音频数据
class AV_EXPORT AudioRender :public Handler
{
	SUPER(Handler)
public:
	AudioRender();
	virtual ~AudioRender();
	virtual int Start();
	virtual void Stop();

	virtual int  EnableSilentMode(bool silent);
	virtual bool IsSilentMode()
	{
		return m_silentMode;
	}

	ULONGLONG GetTick()const
	{
		return mTick;
	}

	virtual int WriteAudio(LPBYTE pAudio, int cbAudio);

	BOOL IsMute()const
	{
		return mMute;
	}
	virtual int SetMute(BOOL bMute = TRUE)
	{
		mMute = bMute;
		return 0;
	}
	virtual BOOL IsRunning()const
	{
		return mRunning;
	}

	void Empty();

	virtual BOOL SetVolume(long lVolume)
	{
		return FALSE;
	}
	virtual BOOL GetVolume(long& lVolume)
	{
		return FALSE;
	}

	DWORD GetTotalBytes()const
	{
		return mTotalBytes;
	}

	//0 is pcm
	//1 is adpcm
	void SetAudoiFormat(int format)
	{
		mAdpcm = (format == 1);
	}
	int GetPcmData(LPBYTE pBuf, int cbSize);
protected:
	void OnCreate();
	int OnProcDataGetter(const std::string& name, std::string& desc);

	int AdjustPcm(LPBYTE pcm, int cbpcm, int level);

	BOOL				mRunning;
	CriticalSection	m_mutexAudio;

	RingBuffer			mRingBuffer;
	int					mRingBufferCacheBytes;

	int					m_nSamplePerSecond;
	int					m_nChannel;
	int					m_nBitsPerSample;
	LPBYTE	m_pDecBuf;
	int		m_cbDecBuf;

	BOOL	mMute;

	ULONGLONG	mTick = 0;
	//m_dwTick用于支持智能停止
	//m_dwTick保存最后一次用户播放音频数据的时间
	//超时没有用户调用WriteAudio,则上层可停止AudioRender

	DWORD	mTotalBytes;

	bool	mAdpcm;
	bool	m_silentMode;
	BYTE	mAudioRtpType;//ulaw为0,alaw为8
	bool	mFirst;//第一次发包时要等待缓存有足够的数据
};
}
}
}