#pragma once
#include "rtsphandler.h"
#include "rtsptextparser.h"
#include "string/textprotocol.h"
#include "libav/videosource.h"
#include "auth/authman.h"
using namespace Bear::Core::Media;
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {



class TalkManager;
//#define _CONFIG_TEST_RTP_PACK

class RTSP_CLASS UploadFileHandler
{
public:
	virtual ~UploadFileHandler() {}
	virtual std::shared_ptr<DataInterface> Prepare(const string& remoteFilePath, const string& tag, DWORD totalBytes);
protected:
	virtual string GetUploadFilePath(const string& tag, const string& remoteFilePath);
};

//XiongWanPing 2016.03.30
class RTSP_CLASS RtspServerHandler :public RtspHandler
{
	SUPER(RtspHandler)

		friend class RtspServer;
public:
	RtspServerHandler();
	virtual ~RtspServerHandler();

	string GetCurrentStreamUrl()const
	{
		return mRtspInfo.mCurrentStreamUrl;
	}

	void AddNotify(const string& msg);
	int OnStartTalk();
	void OnStopTalk();

	virtual void UpdateUserPassword(const string& user, const string&password);
	virtual int PrepareDownloadFile(const string& filePath, DWORD startPos, DWORD& totalBytes);
	virtual int PrepareRecvUploadFile(std::shared_ptr<UploadFileHandler> handler, const string& remoteFilePath, const string& tag, DWORD totalBytes);
	virtual void SetConfig(std::shared_ptr<tagRtspServerConfig> config);

	bool IsPlaying()const
	{
		return mRtspInfo.mPlaying;
	}

	ULONGLONG GetCreateTick()const
	{
		return mCreateTick;
	}

#ifdef  _CONFIG_TEST_RTP_PACK
	static int Test();
	static int TestH265();
#endif
protected:
	void OnCreate();

	void OnPlayStatusChanged();

	virtual int OnRecvRtspCommand(string ack);

	virtual int Process_Options(RtspTextParser& req);
	virtual int Process_Describe(RtspTextParser& req);
	virtual int Process_Setup(RtspTextParser& req);
	virtual int Process_Play(RtspTextParser& req);
	virtual int Process_Pause(RtspTextParser& req);
	virtual int Process_Teardown(RtspTextParser& req);
	virtual int Process_GetParameter(RtspTextParser& req);
	virtual int Process_SetParameter(RtspTextParser& req, TextProtocol& tp);

	string GetVideoSdp(string url);
	string GetVideoSdp(std::shared_ptr<VideoSource> src);
	string GetAudioSdp();

	void OnTimer(long timerId);
	void OnWorkTimer();

	virtual bool IsLive()
	{
		return true;
	}

	void PackVideoH264(std::shared_ptr<VideoFrame>);
	void PackVideoH265(std::shared_ptr<VideoFrame>);
	void PackAudio(LPBYTE audio, int audioBytes);

	bool HasAuth(string auth);
	void OnVideoFeedback(tagRTPHeader *rtp);
	void PreSendVideoRtp(std::shared_ptr<ByteBuffer>& box);

	WORD CalcSeqDelta(WORD serverSeq, WORD clientSeq);

#ifdef _DEBUG
	void SaveRtp(std::shared_ptr<ByteBuffer>& box);
#endif

	void CheckPackUploadFile();
	void OnRecvFile(LPBYTE data, int dataLen, bool eof, bool fileError);
	virtual void OnRecvFileDone(tagRtspFileInfo& info, int error);
	void OnSendFileDone(tagRtspFileInfo& info, int error);

	void OnClose(Channel*);
	void OnDestroy();
	virtual int OnRecvRtpAudioPack(tagRTSPInterleavedHeader *pRtspHeader);

protected:

	struct tagRtspInfo
	{
		tagRtspInfo()
		{
			mVideoEverMaxDelay = 0;
			mClientVideoSeq = 0;

			//说明:为了方便做流量控制,rtp seq仅在最开始初始化为0,后面即使切换video size,seq也是连续的
			//这样便于计算server和client回传的seq差值,见CalcSeqDelta
			//这是符合RF3550规范的
			mAudioRtpSequenceNumber = 0;
			mVideoRtpSequenceNumber = 0;

			mVideoSeqDelta = 0;
			mServerVideoSeq = 0;
			mSupportNotify = false;

			Reset();
		}

		void Reset()
		{
			mSession = 0;
			mPlaying = false;
			mSetupAudio = false;
			mSetupVideo = false;
			mEnableSendNewVideoFrame = true;
			mClientSupportVideoFeedback = false;
			mAudioFrameIndexHasGet = 0;
			mVideoFrameIndexHasGet = 0;
			mAudioFrameCount = 0;
			mVideoFrameCount = 0;
			mVideoMaxDelay = 0;
			mDescribeReqCache.clear();
			mCurrentStreamUrl.clear();

			mVideoBaseTick = ShellTool::GetTickCount64();
			mVideoTickCache = 0;
			mCSeq = 0;
		}

		DWORD	mSession;
		bool	mSetupAudio;
		bool	mSetupVideo;
		bool	mClientSupportVideoFeedback;//客户端是否支持视频反馈
		bool    mEnableSendNewVideoFrame;//是否允许发送新视频帧,用来做流量控制
		bool	mPlaying;
		bool	mSupportNotify;
		DWORD	mAudioFrameIndexHasGet;
		DWORD	mVideoFrameIndexHasGet;
		WORD	mAudioRtpSequenceNumber;
		WORD	mVideoRtpSequenceNumber;
		DWORD	mAudioFrameCount;
		DWORD	mVideoFrameCount;
		WORD	mServerVideoSeq;	//服务端已提交的video rtp seq
		WORD	mClientVideoSeq;	//客户端已确认收到的video rtp seq
		WORD    mVideoSeqDelta;		//供proc使用
		DWORD	mVideoMaxDelay;
		DWORD	mVideoEverMaxDelay;
		string	mDescribeReqCache;
		string	mCurrentStreamUrl;
		UINT		mCSeq;

		ULONGLONG	mVideoBaseTick;
		ULONGLONG	mVideoTickCache;//用来避免两个video frame的timestamp相同或回退,否则客户端ffmpeg录像时av_interleaved_write_frame返回-22错误

	}mRtspInfo;
	ULONGLONG GetVideoRtpTimestamp();

	void CheckVideoFlowControl();

	std::shared_ptr<AudioSource> mAudioSource;
	std::shared_ptr<VideoSource> mVideoSource;

	string CreateVideoSdpFMTP(tagH264Or5BoxInfo& info);
	unsigned removeH264or5EmulationBytes(BYTE* to, unsigned toMaxSize, BYTE const* from, unsigned fromSize);

	void DumpFrames();
#ifdef _CONFIG_TEST_RTP_PACK
	FILE	*mFile;
	bool	mAddRtspHeader;//live555发h.265时采用udp,不带rtsp header,为方便比较，我们也不加rtsp header

	struct tagH265FileSourceInfo
	{
		tagH265FileSourceInfo()
		{
			mFile = NULL;
			mReachEOF = false;
		}

		string	mFilePath;
		FILE		*mFile;
		bool		mReachEOF;
		ByteBuffer	mCacheBox;
	}mH265Info;
#endif

	std::shared_ptr<tagRtspServerConfig> mRtspConfig;
	std::shared_ptr<UserInfo>			mUserInfo;

	tagRtspFileInfo mUploadSink;	//保存对方主动上传的文件
	tagRtspFileInfo mDownloadSource;//对方主动请求下载此文件

	ULONGLONG		mCreateTick;
	string		mCreateTime;

	std::shared_ptr<AudioRender> mAudioRender;
	std::shared_ptr<TalkManager> mTalkManager;
	int mTalkId;//由mTalkManager返回，用来唯一标识对讲
	bool mEnableRtspAudio = true;//供测试使用

	long mTimerProcess = 0;
	long mTimerCheckVideoSpecialFrame = 0;
};

}
}
}
}
