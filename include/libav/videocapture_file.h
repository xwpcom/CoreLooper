#pragma once
#include "videocapture.h"
#include "av_inc.h"

namespace Bear {
namespace Core {
namespace Media {

class FileDemuxer_ffmpeg;

class AV_EXPORT VideoCapture_File :public VideoCapture
{
	SUPER(VideoCapture)
public:
	VideoCapture_File();
	virtual ~VideoCapture_File();

	int Open(string filePath);
	int GetVideoInfo(tagVideoMediaInfo& info);

protected:
	int GetNextFrame();

protected:
	std::shared_ptr<FileDemuxer_ffmpeg> mDemuxer;

};

}
}
}
