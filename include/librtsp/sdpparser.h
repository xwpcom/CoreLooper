#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {


struct tagSdpAudioInfo
{
	bool	bValid;				//只有在bValid为TRUE时,本struct的其他成员才有效
	int		payload_type;		//rtp payload
	int		type;				//
	char	track[32];			//track标识
	char	encoding_name[32];	//PCMU(ULAW)或者AMR
#ifdef _CONFIG_JCO_IPCAM
	char	mSession[16];
#endif

	tagSdpAudioInfo()
	{
		Reset();
	}
	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
};

struct tagSdpVideoInfo
{
	bool	bValid;				//只有在bValid为TRUE时,本struct的其他成员才有效
	int		payload_type;		//rtp payload
	char	encoding_name[32];	//JPEG,mpeg4,h.264,h.265
	DWORD	pixelFormat;		//V4L2_PIX_FMT_H264,V4L2_PIX_FMT_H265
	int		clock_rate;			//90000
	int		nfps;				//帧率
	int		width;				//video width
	int		height;				//video height
	char	track[64];			//track标识
								//char	sprop_parameter_sets[1024];//should be big enough仅在h.264/h.265时有效,
	char	mVPS[128];			//仅h.265有效,存放vps base64字符串
	char	mSPS[128];
	char	mPPS[128];
	//eVideoEncode videoEncode;
	float	m_rangeStart;
	float	m_rangeStop;

	//#ifdef _CONFIG_JCO_IPCAM
	char	mSession[64];//只有jco ipcam需要此项
	 //#endif

	tagSdpVideoInfo()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
		m_rangeStart = 0.0;
		m_rangeStop = 0.0;
	}
};

//XiongWanPing 2016.04.27
class RTSP_CLASS SdpParser
{
public:
	SdpParser(const char *szSDP);
	virtual ~SdpParser();

	int GetAudioInfo(tagSdpAudioInfo& ai);
	int GetVideoInfo(tagSdpVideoInfo& vi);

protected:
	char m_szSDP[2048];
};

}
}
}
}
