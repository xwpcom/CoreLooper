#pragma once
namespace Bear {
namespace Core {
namespace Media {

#define H264_GET_NALU_TYPE(x) ((x)&0x1F)

//h.264 nalu type
#define NALU_SLICE		1
#define NALU_IDR		5
#define NALU_SEI		6
#define NALU_SPS		7
#define NALU_PPS		8

//*
//和libx264冲突
//h.265 nalu type
enum NALUnitType {
	NAL_TRAIL_N = 0,
	NAL_TRAIL_R = 1,
	NAL_TSA_N = 2,
	NAL_TSA_R = 3,
	NAL_STSA_N = 4,
	NAL_STSA_R = 5,
	NAL_RADL_N = 6,
	NAL_RADL_R = 7,
	NAL_RASL_N = 8,
	NAL_RASL_R = 9,
	NAL_BLA_W_LP = 16,
	NAL_BLA_W_RADL = 17,
	NAL_BLA_N_LP = 18,
	NAL_IDR_W_RADL = 19,
	NAL_IDR_N_LP = 20,
	NAL_CRA_NUT = 21,
	NAL_VPS = 32,
	NAL_SPS = 33,
	NAL_PPS = 34,
	NAL_AUD = 35,
	NAL_EOS_NUT = 36,
	NAL_EOB_NUT = 37,
	NAL_FD_NUT = 38,
	NAL_SEI_PREFIX = 39,
	NAL_SEI_SUFFIX = 40,
};
//*/

#ifndef v4l2_fourcc
#define v4l2_fourcc(a,b,c,d)\
	(((unsigned int)(a)<<0)|((unsigned int)(b)<<8)|((unsigned int)(c)<<16)|((unsigned int)(d)<<24))
#endif

#ifndef V4L2_PIX_FMT_H264
#define V4L2_PIX_FMT_H264  v4l2_fourcc('H','2','6','4')
#endif

#ifndef V4L2_PIX_FMT_H265
#define V4L2_PIX_FMT_H265  v4l2_fourcc('H','2','6','5')
#endif

#ifndef V4L2_PIX_FMT_MJPG
#define V4L2_PIX_FMT_MJPG  v4l2_fourcc('M','J','P','G')
#endif

struct tagVideoMediaInfo
{
	tagVideoMediaInfo()
	{
		Reset();
	}

	void Reset()
	{
		mPixelFormat = 0;
		mWidth = 0;
		mHeight = 0;
	}

	static const char *GetPixelFormat(DWORD fmt)
	{
		if (fmt == V4L2_PIX_FMT_H264)	return "H.264";
		if (fmt == V4L2_PIX_FMT_H265)	return "H.265";
		return "Unknown";
	}

	DWORD	mPixelFormat;//V4L2_PIX_FMT_MJPEG,V4L2_PIX_FMT_H264
	UINT	mWidth;
	UINT	mHeight;
};

}
}
}