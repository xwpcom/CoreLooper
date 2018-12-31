#pragma once
class RtspServerHandler;
#include "rtspserverhandler.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {

//XiongWanPing 2016.09.14
//文件上传管理器,管理RTSP上传文件
class RTSP_CLASS RtspUploadManager :public Handler
{
	SUPER(Handler);
public:
	RtspUploadManager();
	~RtspUploadManager();

	void SetUploadFileHandler(std::shared_ptr<UploadFileHandler> handler)
	{
		mHandler = handler;
	}

	virtual int PrepareRequestUpload(std::shared_ptr<RtspServerHandler> context, const string& tag, const string& remoteFilePath, DWORD totalBytes);
	virtual void OnRecvFileDone(std::shared_ptr<RtspServerHandler> context, const string& tag, const string& filePath, int error);

	sigslot::signal4<RtspUploadManager*, const string&, const string&, int>	SignalUploadDoneEvent;
protected:
	std::weak_ptr<RtspServerHandler> mContext;
	std::shared_ptr<UploadFileHandler> mHandler;
};

}
}
}
}
