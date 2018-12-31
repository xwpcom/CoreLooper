#pragma once

#include "rtsphandler.h"
#include "sdpparser.h"
#include "rtsptextparser.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {


//must sync with CameraStreamStatus
enum eRtspStreamStatus
{
	eRtspStreamStatus_None,
	eRtspStreamStatus_PasswordInvalid,//密码无效
	eRtspStreamStatus_UnSupported,//不支持此设备
	eRtspStreamStatus_Connected, //通道创建成功，可提交ajax命令或执行rtsp指令了
	eRtspStreamStatus_Ready,	//可以play	
	eRtspStreamStatus_Playing,	//可以pause	
	eRtspStreamStatus_Paused,	//可以play
	eRtspStreamStatus_Closed,	//已关闭
};

struct tagRtspStatus
{
	tagRtspStatus();
	void Reset();

	void OnSentOptions();
	void OnSentDescribe();
	void OnSentSetupAudio();
	void OnSentSetupVideo();
	void OnSentPlay();
	void OnSentPause();
	void OnSentTeardown();

	bool	m_sent_options;
	bool	m_sent_describe;
	bool	m_sent_setup_audio;
	bool	m_sent_setup_video;
	bool	m_sent_play;
	bool	m_sent_pause;
	bool	m_sent_teardown;

	bool	mOptionsAcked;
	bool	mDescribeAcked;

	bool	mUserPlay;//上层app是否希望play,sdk会尽量满足app的要求
	bool	mPaused;
	bool	mEnableKeepAlive;//是否启用保活,Release版应该启用
	bool	mEnableVideoFeedback;//是否支持视频反馈，用来做流量控制
	eRtspStreamStatus	mStreamStatus;
	eRtspError			mError;
	string			mContentBase;

	tagSdpAudioInfo		mAI;
	tagSdpVideoInfo		mVI;
	tagVodProgressInfo	m_vodProgressInfo;

	string			mRequestCache;
	string			mAckCache;
};

struct tagRtspConnectInfo
{
	string	mDeviceAddress;
	int			mPort;
	string	mUser;
	string	mPassword;
};

class RtspClientHandler;
class RTSP_CLASS IRtspClientHandlerCB
{
public:
	virtual ~IRtspClientHandlerCB() {}

	//接收到完整的视频帧后会调用本接口
	virtual int OnRtspVideoFrame(
		RtspClientHandler *handler,
		DWORD fmt,
		LPBYTE pFrame,
		int cbFrame,
		DWORD dwTimeStamp,
		tagRTPExtension *rtpExt
	) = 0;

	//接收到完整的音频帧后会调用本接口
	virtual int OnRtspAudioFrame(
		RtspClientHandler *handler,
		DWORD fmt,
		LPBYTE pFrame,
		int cbFrame,
		DWORD dwTimeStamp
	) = 0;

	/*
	//接收到下载文件的数据时会调用本接口
	virtual int OnDemuxDownloadFile(RtspClientHandler *handler, LPBYTE data, int dataLen, bool eof, bool fileError) = 0;
#ifdef _CONFIG_JCO_DEVICE
	virtual int OnAlarmMessage(IRtspClientHandler *handler, string msg) = 0;
#endif

	virtual int OnRtspError(
		RtspClientHandler *handler,
		int err
	) = 0;

	virtual int Output(RtspClientHandler *handler, const LPBYTE pData, int cbData) = 0;
	virtual void OnRtspMsg(RtspClientHandler *handler, eRtspMsg msg) {};
	*/
};

//XiongWanPing 2016.04.26
class RTSP_CLASS RtspClientHandler :public RtspHandler
{
	SUPER(RtspHandler)
public:
	RtspClientHandler();
	virtual ~RtspClientHandler();

	string GetCurrentStreamUrl()const
	{
		ASSERT(IsMyselfThread());
		return mCurrentStreamUrl;
	}

	struct tagUploadDownloadFileInfo
	{
	public:
		tagUploadDownloadFileInfo()
		{
			Reset();
		}

		void Reset()
		{
			mRequesting = false;
			mTransfering = false;
			mTotalBytes = 0;
			mDoneBytes = 0;
		}

		bool	mRequesting;//正在请求，还没收到回复
		bool	mTransfering;
		ULONG	mTotalBytes;
		ULONG	mDoneBytes;
	};

	virtual int GetDownloadFileInfo(tagUploadDownloadFileInfo& info);
	virtual int GetUploadFileInfo(tagUploadDownloadFileInfo& info);
	virtual void OnSendFileDone(tagRtspFileInfo& info, int error);

	virtual long RequestUploadFile(const string& filePath, const string& tag);
	virtual long RequestDownloadFile(const string& remoteFilePath, const string& saveAsLocalPath);

	void SetCB(IRtspClientHandlerCB	*cb)
	{
		ASSERT(IsMyselfThread());
		mHandlerCB = cb;
	}

	virtual int AddAjaxRequest(const char *url);
	virtual int AddAjaxRequest(const string& url)
	{
		if (!url.empty())
		{
			return AddAjaxRequest(url.c_str());
		}

		return -1;
	}
	virtual int AddRtspSetParamRequest(const char *pszReq);
	virtual int AddRtspSetParamRequest(const string& req)
	{
		if (!req.empty())
		{
			return AddRtspSetParamRequest(req.c_str());
		}

		return -1;
	}

	virtual int AddTalkAudio(LPBYTE pAudio, int cbAudio);

	const tagSdpAudioInfo& GetAudioInfo()const
	{
		return mRtspStatus.mAI;
	}

	const tagSdpVideoInfo& GetVideoInfo()const
	{
		return mRtspStatus.mVI;
	}

	virtual void SetConfig(std::shared_ptr<tagRtspClientConfig> config)
	{
		mRtspConfig = config;
	}

	void SetConnectInfo(const tagRtspConnectInfo& info)
	{
		mConnectInfo = info;
	}
	virtual int SetStreamUrl(string url, bool singleStreamDevice = true);

	virtual int Play();
	virtual int Pause();
	virtual int Teardown();

	eRtspStreamStatus GetStreamStatus()const
	{
		return mRtspStatus.mStreamStatus;
	}
	sigslot::signal2<RtspClientHandler*, eRtspStreamStatus>			SignalStreamStautusChanged;
	sigslot::signal4<RtspClientHandler*, string, int, string>	SignalAjaxAck;
	sigslot::signal2<RtspClientHandler*, const string&>			SignalEvent;

	int OnRtspError(eRtspError err);
protected:
	void OnCreate();
	virtual void OnConnect(Channel*endPoint, long error, ByteBuffer *pBox, Bundle	*extraInfo);

	virtual long RequestDownloadFile_Impl(const string& remoteFilePath, const string& saveAsLocalPath);
	virtual int AddAjaxRequest_Impl(const char *url);

	virtual int OnRecvRtspCommandAck(string req, string ack);
	virtual int OnRecvRtpAudioPack(tagRTSPInterleavedHeader *pRtspHeader);
	virtual int OnRecvRtpVideoPack(tagRTSPInterleavedHeader *pRtspHeader);

	virtual int Process_Options_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int Process_Describe_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int Process_Setup_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int Process_GetParameter_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int Process_SetParameter_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int Process_Play_Ack(RtspTextParser& req, RtspTextParser& ack);
	virtual int OnRecvRtspCommand(string ack);

	virtual LRESULT OnMessage(UINT msg, WPARAM wp, LPARAM lp);
	virtual void FireEvent(const string& msg);

	string GetUserAgent();
	void SetStreamStatus(eRtspStreamStatus status);

	int SendDescribe();
	int SendSetupVideo();
	int SendSetupAudio();
	int SendVideoFeedback(tagRTPHeader *rtp);

	void OnTimer(long timerId);

	void CheckPackUploadFile();
	void OnRecvFile(LPBYTE data, int dataLen, bool eof, bool fileError);
	void OnRecvFileDone(tagRtspFileInfo& info, int error);
	void AddNotify(const string& msg);

protected:
	tagRtspConnectInfo				mConnectInfo;
	std::shared_ptr<tagRtspClientConfig>	mRtspConfig;

	ByteBuffer						mVideoInbox;

	tagRtspStatus					mRtspStatus;
	string						mStreamUrl;
	bool							mSingleStreamDevice;
	string						mCurrentStreamUrl;//由rtsp server返回的当前实际stream url

	struct tagAuthRealmInfo
	{
		tagAuthRealmInfo()
		{
			Reset();
		}

		void Reset()
		{
			mBasicRealm = "";
			mDigestRealm = "";
			mNonce = "";
			mEnable = false;
			mHasUseAuth = false;
		}

		//支持base64和digest
		string	mBasicRealm;
		string	mDigestRealm;
		string	mNonce;
		bool		mEnable;
		bool		mHasUseAuth;
	};

	tagAuthRealmInfo mAuthRealmInfo;

	struct tagRtpVideoInfo
	{
		tagRtpVideoInfo()
		{
			Reset();
		}

		void Reset()
		{
			mRtpExt.Reset();

			fCurPacketNALUnitType = 0;
			fExpectDONFields = false;
			fCurrentPacketBeginsFrame = false;
			fCurrentPacketCompletesFrame = false;
			resultSpecialHeaderSize = 0;
			fPreviousNALUnitDON = 0;
			fCurrentNALUnitAbsDon = (ULONGLONG)(~0);
			mSpecialBytesOffset = 2;
			mVideoDemuxFormat = 0;
		}

		bool			fExpectDONFields;
		bool			fCurrentPacketBeginsFrame;
		bool			fCurrentPacketCompletesFrame;
		BYTE			fCurPacketNALUnitType;
		BYTE			mSpecialBytesOffset;
		BYTE			resultSpecialHeaderSize;
		ULONGLONG		fPreviousNALUnitDON;
		ULONGLONG		fCurrentNALUnitAbsDon;
		DWORD			mVideoDemuxFormat;
		tagRTPExtension	mRtpExt;
	}mRtpVideoInfo;

	int processSpecialHeader_H265(LPBYTE frame, int frameLen);
	void computeAbsDonFromDON(WORD DON);

	void SendKeepAlive();

	IRtspClientHandlerCB	*mHandlerCB;

	tagRtspFileInfo mUploadSource;	//本方主动上传此文件
	tagRtspFileInfo mDownloadSink;	//保存本方主动下载的文件

	long mTimerTest = 0;
	long mTimerKeepAlive = 0;
	long mTimerRetry = 0;
};

}
}
}
}
