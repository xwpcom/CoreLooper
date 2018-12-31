#pragma once

struct AVFormatContext;
struct AVStream;
#include "MediaRecorderCB.h"

namespace Bear {
namespace Core {
namespace Media {

#ifdef _MSC_VER_DEBUG
//#define _CONFIG_TEST_RECORDER	//ffmpeg录像有时报错，定义此宏来dump所有输入数据，便于重现问题
#endif

//XiongWanPing 2016.08.08
//采用ffmpeg来做录像，目前支持h.264/h.265和ulaw
//audio固定为ulaw,8khz,16bit,mono
//video支持h.264和h.265
class AV_EXPORT MediaRecorder
{
public:
	MediaRecorder();
	virtual ~MediaRecorder();

	int InitH264Recorder(string filePath, bool hasAudio = true);
	int InitH265Recorder(string filePath, bool hasAudio = true);

	int WriteVideo(LPBYTE frame, int frameLen, ULONGLONG timestamp);
	int WriteAudio(LPBYTE frame, int frameLen, ULONGLONG timestamp);

	//切换之前要调用SetNameCB提供命名
	int MarkSwitchFile()
	{
		ASSERT(mCB);
		mInfo.mMarkSwitchFile = true;
		return 0;
	}

	void SetCB(MediaRecorderCB* cb)
	{
		mCB = cb;
	}

	void Close();

	bool HasUsableVideoFrame();

	void EnableDumpInfo()
	{
		mEnableDumpInfo = true;
	}

	void DisableDumpInfo()
	{
		mEnableDumpInfo = false;
	}

protected:
	int InitRecorder(DWORD videoCodecId, string filePath, bool hasAudio = true);

	bool IsInited()const;
	bool IsH264()const;
	bool IsH265()const;
	bool IsKeyFrame(BYTE nalu);

	bool IsVPS(BYTE nal_unit_type)const;
	bool IsSPS(BYTE nal_unit_type)const;
	bool IsPPS(BYTE nal_unit_type)const;

	int WriteH264(LPBYTE frame, int frameLen, ULONGLONG timestamp);
	int WriteH265(LPBYTE frame, int frameLen, ULONGLONG timestamp);

	int SwitchFile();

protected:
	struct tagInfo
	{
	public:
		tagInfo()
		{
			Reset();
		}

		void Reset();

		AVFormatContext *oc;
		AVStream *mStreamVideo;
		AVStream *mStreamAudio;
		int mAudioIndex;
		int mVideoIndex;

		ByteBuffer mSps;
		ByteBuffer mPps;
		ByteBuffer mVps;//h.265 only

		ByteBuffer	mVideoExtraData;//h.264和h.265都需要
		string	mFilePath;
		DWORD		mVideoCodecId;//AVCodecID
		CSize		mVideoSize;
		bool		mMarkSwitchFile;	//在遇到下一个I帧时，切换录制到新文件
		ULONGLONG	mAudioTimeStampBase;
		ULONGLONG	mVideoTimeStampBase;
		ULONGLONG	mPtsCache = 0;//保证传给ffmpeg的video timestamp是单调递增的
	}mInfo;

	MediaRecorderCB *mCB;

#ifdef _CONFIG_TEST_RECORDER
	struct tagTestInfo
	{
		tagTestInfo()
		{
			//mTestTimes = -1;
			Reset();
		}

		void Reset()
		{
		}

		//int mTestTimes;
	}mTestInfo;
#endif

	bool mEnableDumpInfo = false;
};

}
}
}
