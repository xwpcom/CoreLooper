#pragma once
namespace Bear {
namespace Core {
namespace Media {
//XiongWanPing 2011.01.16
//当PCM为8000hz,双声道时,每秒16KB audio,每块640字节时,每秒为25个块
//AudioInbox给每个块作一个唯一标识,方便多个独立的client获取audio数据
//采用一个固定尺寸的数组来保存读取的pcm数据,并做循环覆盖
//client在读取audio数据时,要传递上一次读取时的下标,AudioInbox根据此下标返回下一块数据

#define AUDIO_BLOCK_SIZE	320
class AV_EXPORT AudioInbox
{
public:
	AudioInbox(void);
	virtual ~AudioInbox(void);
	int WriteAudio(LPBYTE pAudio, int cbAudio);
	int ReadAudio(LPBYTE pAudio, int cbAudio, DWORD& dwIndex);

	int GetAudioBlockSize()const
	{
		return AUDIO_BLOCK_SIZE;
	}

	void Empty();

protected:
	int InitAudioBlock();
	int AddAudioBlock(LPBYTE pAudioBlock, int cbAudioBlock);

	struct tagAudioBlock
	{
		tagAudioBlock()
		{
			dwIndex = 0;
		}

		DWORD	dwIndex;
		BYTE	audio[AUDIO_BLOCK_SIZE];
	};

	CriticalSection				m_cs;
	tagAudioBlock	*m_pAudioBlock = nullptr;
	int				m_cbAudioBlock = 0;//m_pAudioBlock中item个数

	int				m_nNextWriteAudioBlock;
	//下一次写新数据时的audio block下标,有效范围为[0~m_cbAudioBlock)

	DWORD			m_dwNextWriteAudioBlockIndex;//audio block id
	//单调递增,为0表示无效的audio块

	//从PC声卡设备读取的PCM数量可能不是AUDIO_BLOCK_SIZE的整数倍,
	//采用m_audioCache来保存边角料
	BYTE	m_audioCache[AUDIO_BLOCK_SIZE];
	int		m_nAudioCache;//m_audioCache中的有效audio字节数
};

}
}
}
