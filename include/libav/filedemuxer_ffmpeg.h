#pragma once
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
//#include <libavutil/buffer.h>
#include <libswresample/swresample.h>
}

#include "av_inc.h"

namespace Bear {
namespace Core {
namespace Media {

//XiongWanPing 2016.04.14
class AV_EXPORT FileDemuxer_ffmpeg
{
public:
	FileDemuxer_ffmpeg();
	virtual ~FileDemuxer_ffmpeg();

	int Test();

	int Open(string filePath);
	void Close();

	int GetVideoInfo(tagVideoMediaInfo& info);
	int GetNextVideoFrame(ByteBuffer& box, bool& isKeyFrame);

protected:
	int GetNextParamSetBox(ByteBuffer& box);

	int OpenCodecContext(AVMediaType type, int& stream_idx);
	int ExtractSprops(const char *sdpInfo);

	struct tagVideoInfo
	{
		tagVideoInfo()
		{
			mAVStream = nullptr;
			mCodecContext = nullptr;
			Reset();
		}

		void Reset()
		{
			mStreamIndex = -1;
			mFrameIndex = -1;
			mParamSetReturnCount = 0;
			mParamSetReturnDone = false;
			mAVStream = nullptr;
			mCodecContext = nullptr;

			mVPS.clear();
			mSPS.clear();
			mPPS.clear();
		}

		bool IsValid()const
		{
			return mStreamIndex != -1;
		}
		int mStreamIndex;
		int mFrameIndex;
		AVStream *mAVStream;
		AVCodecContext *mCodecContext;

		ByteBuffer mVPS;	//only for h.265
		ByteBuffer mSPS;	//for h.264 & h.265
		ByteBuffer mPPS;	//for h.264 & h.265
		int mParamSetReturnCount;//GetNextVideoFrame()最开始应该返回vps,sps和pps
		bool mParamSetReturnDone;
	}mVideoInfo;

	struct tagAudioInfo
	{
		tagAudioInfo()
		{
			mAVStream = nullptr;
			mCodecContext = nullptr;
			Reset();
		}

		void Reset()
		{
			mStreamIndex = -1;
			mFrameIndex = -1;
			mAVStream = nullptr;
			mCodecContext = nullptr;
		}

		bool IsValid()const
		{
			return mStreamIndex != -1;
		}

		int mStreamIndex;
		int mFrameIndex;
		AVStream *mAVStream;
		AVCodecContext *mCodecContext;
	}mAudioInfo;

	AVFormatContext *mFormatContext;
	AVPacket	mAVPacket;

	string	mFilePath;
	bool		mEnableRewind;
};

}
}
}
