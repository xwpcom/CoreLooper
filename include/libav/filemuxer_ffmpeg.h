#pragma once
namespace Bear {
namespace Core {
namespace Media {

//XiongWanPing 2016.05.10
//采用ffmpeg做录像
class AV_EXPORT FileMuxer_ffmpeg
{
public:
	FileMuxer_ffmpeg();
	virtual ~FileMuxer_ffmpeg();
#ifdef _DEBUG
	static int Test();
	static int TestH265();
	static int TestH264();
#endif
};

}
}
}
