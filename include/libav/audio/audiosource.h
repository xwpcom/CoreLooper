#pragma once
#include "audioinbox.h"
namespace Bear {
namespace Core {
namespace Media {


#define PCM_BLOCK_SIZE		640
#define ULAW_BLOCK_SIZE		320

//XiongWanPing 2011.06.12
//负责从硬件设备采集audio数据并供其他组件使用
class AV_EXPORT AudioSource :public Handler
{
	SUPER(Handler);
public:
	AudioSource();
	virtual ~AudioSource();

	void OnCreate();

	virtual int Start();
	virtual void Stop();

	virtual int ReadAudio(LPBYTE pAudio, int cbAudio, DWORD& dwIndex);

	BOOL IsRunning() const
	{
		return mRunning;
	}

	ULONGLONG GetTick()const
	{
		return mTick;
	}

	void Empty();

	virtual BOOL SetAudioDevice(std::string szDev)
	{
		return FALSE;
	}

	virtual int Input(LPBYTE pcm, int pcmLen)
	{
		return OnReadPcm(pcm, pcmLen);
	}
protected:
	void OnTimer(long timerId);
	BOOL	mRunning;

	int		m_nBitsPerSample;
	int		m_nChannel;
	int		m_nSamplePerSecond;

	//从PC声卡设备读取的PCM数量可能不是PCM_BLOCK_SIZE的整数倍,
	//采用m_CachePcm来保存边角料
	BYTE	m_CachePcm[PCM_BLOCK_SIZE];
	int		m_nCachePcm;//m_CachePcm中的有效audio字节数

	virtual BOOL OnReadPcm(LPBYTE pPcm, int cbPcm);
	BOOL AddPcmBlock(LPBYTE pPcm, int cbPcm);
	CriticalSection				m_cs;

	ULONGLONG	mTick;
	//mTick用于支持智能停止
	//mTick保存最后一次用户调用ReadPcmBlock()的GetTickCount(),
	//用来检测是否有用户需要AudioSource_Windows提供的音频数据,
	//超时没有用户调用ReadPcmBlock,则CDxAudiInEx自动停止工作线程

	AudioInbox	mAudioIn;	//保存audio数据
	long mTimerSmartStop = 0;
};

}
}
}
