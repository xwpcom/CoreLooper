#pragma once

namespace Bear {
namespace Core {
namespace Media {

class MediaRecorder;
//当切换录像文件时，通过本接口来获取录像文件名
//应用场景:
//为了保证不丢失视频帧，仅能在I帧切换,一般来说，每1秒~4秒或更长时间才有一个I帧
//录像文件一般按时间来命名,这导致无法提前获知I帧对应的时间
//故此采用回调来取文件名
class MediaRecorderCB
{
public:
	virtual ~MediaRecorderCB() {}
	virtual int GetMediaRecorderFilePath(MediaRecorder *recorder, string& filePath) = 0;
	virtual void OnStartRecord(MediaRecorder *recorder, string filePath, int error) = 0;
	virtual void OnStopRecord(MediaRecorder *recorder, string filePath, int error) = 0;
};

}
}
}
