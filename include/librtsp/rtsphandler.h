#pragma once
#include "rtsp_inc.h"
#include "net/speedcounter.h"
#include "file/transferfileinfo.h"

using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net::Http;
using Bear::Core::UserInfo;
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {


using Media::AudioRender;

#define _RTP_FILE_CHANNEL				100		//文件固定采用rtp通道100
#define _RTP_VIDEO_FEEDBACK_CHANNEL		101		//视频反馈通道

enum eRtpFileFlag
{
	eRtpFileFlag_EOF = 0x0001,	//文件结束
	eRtpFileFlag_Error = 0x0002,		//出错
};


struct tagRtspFileInfo
{
	tagRtspFileInfo()
	{
		Reset();
	}
	void Reset()
	{
		mTotalBytes = 0;
		mDoneBytes = 0;
		mDataContainer = nullptr;
		mWorking = false;
		mEOF = false;
		mError = false;
		mTag.clear();
	}

	string GetFilePath()
	{
		if (mDataContainer)
		{
			return mDataContainer->GetFilePath();
		}

		return "";
	}

	DWORD mTotalBytes;
	DWORD mDoneBytes;//has send or recv
	std::shared_ptr<DataInterface>	mDataContainer;
	string			mTag;
	bool				mWorking;
	bool				mEOF = false;
	bool				mError = false;
	bool IsOpen()const
	{
		return mDataContainer != nullptr;
	}
	bool IsWorking()const
	{
		return IsOpen() && mWorking && !mEOF && !mError;
	}

	//返回值:从0到100
	int GetPercent()
	{
		if (mTotalBytes > 0)
		{
			return (int)(mDoneBytes * 100.0 / mTotalBytes);
		}

		return 0;
	}
};

//XiongWanPing 2016.04.26
//rtsp handler基类
class RTSP_CLASS RtspHandler :public Handler
{
	SUPER(Handler)
public:
	RtspHandler();
	virtual ~RtspHandler();

	static CSize String2Size(string sz);
	virtual bool HasAuth(string auth);

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*endPoint, long error, ByteBuffer *pBox, Bundle	*extraInfo);

	std::shared_ptr<Channel> mChannel;
	void SetAudioRender(std::shared_ptr<Media::AudioRender> audioRender);
	struct tagSpeedInfo
	{
		tagSpeedInfo()
		{
			Reset();
		}

		void Reset()
		{
			mVideoFps = 0;
			mUp = 0;
			mDown = 0;

			DWORD nc, interval;
			mCounterUp.Reset(nc, interval);
			mCounterDown.Reset(nc, interval);
			mCounterUp.Reset(nc, interval);
		}

		int							mVideoFps;
		int							mUp;		//上行速度,bytes/second
		int							mDown;		//下行速度,bytes/second
		SpeedCounter				mCounterUp;	//上行计速
		SpeedCounter				mCounterDown;	//下行计速
		SpeedCounter				mCounterVideoFps;
	};

	void GetSpeedInfo(tagSpeedInfo& info)
	{
		info = mSpeedInfo;
	}
protected:
	void OnCreate();
	void OnDestroy();
	void OnTimer(long timerId);

	virtual int OnRecvRtpVideoPack(tagRTSPInterleavedHeader *pRtspHeader);
	virtual std::shared_ptr<ByteBuffer> GetNextOutByteBuffer();
	virtual int OnError(string desc);
	virtual bool IsError()const
	{
		return mError;
	}

	virtual int ParseInbox();
	virtual int OnRecvRtpPack(tagRTSPInterleavedHeader *pRtspHeader);
	virtual int OnRecvRtpAudioPack(tagRTSPInterleavedHeader *pRtspHeader);
	virtual int OnRecvRtspCommandAck(string ack);
	virtual int OnRecvRtspCommandAck(string req, string ack);
	virtual int OnRecvRtspCommand(string ack);
	virtual void OnRecvVideoFrame(LPBYTE frame, int frameLen);
	virtual void PreSendVideoFrame(LPBYTE frame, int frameLen);

	virtual void CheckSend();
	virtual void CheckPackUploadFile();
	virtual void CheckPackUploadFileHelper(tagRtspFileInfo& info);
	virtual void OnRecvFile(LPBYTE data, int dataLen, bool eof, bool fileError);
	virtual void OnRecvFileHelper(tagRtspFileInfo& info, LPBYTE data, int dataLen, bool eof, bool fileError);
	virtual void OnRecvFileDone(tagRtspFileInfo& info, int error);
	virtual void OnSendFileDone(tagRtspFileInfo& info, int error);

	virtual int AddRtspAck(const string& text);
	virtual int AddRtspRequest(const string& text);
	virtual int AddRtspCommand(const string& text);

	virtual void OnVideoFeedback(tagRTPHeader *rtp);
	virtual void PreSendVideoRtp(std::shared_ptr<ByteBuffer>& box);

	ByteBuffer					mInbox;
	bool						mError;//protocol/parse error
	bool						mClosed;
	bool						mEnableKeepAlive;//是否启用保活,Release版应该启用
	std::shared_ptr<ByteBuffer>		mOutbox;
	tagSpeedInfo mSpeedInfo;
	void RefreshSpeed();


	//发包规则:
	//.当mOutbox中有数据时先完mOutbox中的数据
	//.当mOutbox为空时,按如下优先级依次从mOutPendingXXX中取出下一个要发送的ByteBuffer给mOutbox,然后发送
	//可保证以单个rtp包为边界,在同一videoFrame的多个rtp frame之间能穿插audio,改进audio的实时性
	std::list<std::shared_ptr<ByteBuffer>>   mOutPending_RtspCommands;	//有待发送的rtsp命令数据,包括ajax,最高优先级
	std::list<std::shared_ptr<ByteBuffer>>   mOutPending_AudioRtps;		//有待发送的audio帧,优先级比video高
	std::list<std::shared_ptr<ByteBuffer>>   mOutPending_VideoRtps;		//有待发送的video帧
	std::list<std::shared_ptr<ByteBuffer>>   mOutPending_File;			//有待发送的文件数据帧,最低优先级

	UINT						mCSeq;
	std::map<int, string>	mMapRequests;

	ULONGLONG					mKeepAliveTick;
	std::shared_ptr<Media::AudioRender>		mAudioRender;

#ifdef _MSC_VER_DEBUG
	DumpFile mDumpFile;
	void Dump(string text);
#endif

	long mTimerCheckKeepAlive = 0;
	long mTimerRefreshSpeed = 0;
};

}
}
}
}
