#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {


class RtspServerHandler;

//XiongWanPing 2016.10.06
//文件下载管理器,管理RTSP下载文件
class RTSP_CLASS RtspDownloadManager :public Handler
{
public:
	RtspDownloadManager();
	~RtspDownloadManager();

	virtual int PrepareRequestDownload(std::shared_ptr<RtspServerHandler> context, const string& filePath, DWORD startPos, DWORD& totalBytes);
	virtual void OnSendFileDone(std::shared_ptr<RtspServerHandler> context, int error);

protected:
	std::weak_ptr<RtspServerHandler> mContext;
};

}
}
}
}
