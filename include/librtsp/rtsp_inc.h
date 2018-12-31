#pragma once
#include "libav/av_inc.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

using namespace Bear::Core::Net::Http;
using Bear::Core::Net::Http::AjaxCommandHandler;

namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {


struct tagRTSPInterleavedHeader
{
#ifdef _DEBUG
	tagRTSPInterleavedHeader()
	{
		//make sure it's 4 bytes
		ASSERT(sizeof(*this) == 4);
	}
#endif

	BYTE	Magic;
	BYTE	Channel;

	//raw length is net order,so protect it by Length()
	WORD	Length()
	{
		return ntohs(length);
	}

	void set_length(WORD len)
	{
		length = htons(len);
	}

protected:
	WORD	length;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct tagRTPHeader
{
#ifdef _DEBUG
	tagRTPHeader()
	{
		ASSERT(sizeof(*this) == 12);
	}
#endif

	//只需要支持小端
	//BYTE	flag;
	//BYTE	payload_type;
#ifdef _CONFIG_BIG_ENDIAN
	BYTE		version : 2;				/* protocol version */
	BYTE		padding : 1;				/* padding flag */
	BYTE		x : 1;				/* header extension flag */
	BYTE		cc : 4;				/* CSRC count */

	/* byte 1 */
	BYTE		marker : 1;				/* marker bit */
	BYTE		payload_type : 7;				/* payload type */
#else
	BYTE		cc : 4;				/* CSRC count */
	BYTE		x : 1;				/* header extension flag */
	BYTE		padding : 1;				/* padding flag */
	BYTE		version : 2;				/* protocol version */

	/* byte 1 */
	BYTE		payload_type : 7;				/* payload type */
	BYTE		marker : 1;				/* marker bit */
#endif

	DWORD timestamp()
	{
		return ntohl(Timestamp);
	}

	void set_timestamp(DWORD ts)
	{
		Timestamp = htonl(ts);
	}

	void set_ssid(DWORD ssid)
	{
		Sync_sid = htonl(ssid);
	}

	DWORD ssid()
	{
		return ntohl(Sync_sid);
	}

	WORD sequence_number()
	{
		return ntohs(Sequence_number);
	}

	void set_sequence_number(WORD s)
	{
		Sequence_number = htons(s);
	}

protected:

	WORD	Sequence_number;
	DWORD	Timestamp;
	DWORD 	Sync_sid;//sync source id
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

//maygion 2008~
#define MAYGION_RTP_EXT_ID	0x2008

//ONVIF-Streaming-Spec-v221.pdf page26
struct tagRTPExtension
{
	tagRTPExtension()
	{
		ASSERT((sizeof(*this) % sizeof(DWORD)) == 0);

		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(*this));
		m_extId = htons(MAYGION_RTP_EXT_ID);

		int len = (sizeof(*this) - 4) / sizeof(DWORD);
		m_length = htons(len);
	}

	void SetDuration(DWORD duration)
	{
		m_duration = htonl(duration);
	}

	DWORD GetDuration()const
	{
		return ntohl(m_duration);
	}

	void set_extId(WORD extId)
	{
		m_extId = htons(extId);
	}
	WORD extId()const
	{
		return ntohs(m_extId);
	}

	//only use struct tm year,month,day,hour,minute,second
	void set_ntpTime(const struct tm& tmRtp)
	{
		//自定义格式,总共7字节
		//还有一字节可用来做毫秒,目前没使用
		int year = 1900 + tmRtp.tm_year;	//WORD
		int month = tmRtp.tm_mon + 1;		//BYTE
		int day = tmRtp.tm_mday;			//BYTE
		int hour = tmRtp.tm_hour;			//BYTE
		int minute = tmRtp.tm_min;		//BYTE
		int second = tmRtp.tm_sec;		//BYTE

#ifdef _MSC_VER_DEBUGx
		DT("cs#vod rtp time=[%d.%02d.%02d %02d:%02d:%02d]",
			year, month, day,
			hour, minute, second
		);
#endif

		BYTE ms = 0;

		DWORD ntpTime0 = (year << 16) | (month << 8) | day;
		DWORD ntpTime1 = (hour << 24) | (minute << 16) | (second << 8) | ms;

		m_ntpTime0 = htonl(ntpTime0);
		m_ntpTime1 = htonl(ntpTime1);
	}

	//only use struct tm year,month,day,hour,minute,second
	const struct tm get_ntpTime()const
	{
		DWORD ntpTime0 = ntohl(m_ntpTime0);
		DWORD ntpTime1 = ntohl(m_ntpTime1);

		int year = ntpTime0 >> 16;
		int month = (ntpTime0 >> 8) & 0xFF;
		int day = ntpTime0 & 0xFF;
		int hour = (ntpTime1 >> 24) & 0xFF;
		int minute = (ntpTime1 >> 16) & 0xFF;
		int second = (ntpTime1 >> 8) & 0xFF;
		int ms = (ntpTime1) & 0xFF;

		struct tm tmRtp;
		memset(&tmRtp, 0, sizeof(tmRtp));

		tmRtp.tm_year = year - 1900;
		tmRtp.tm_mon = month - 1;
		tmRtp.tm_mday = day;

		tmRtp.tm_hour = hour;
		tmRtp.tm_min = minute;
		tmRtp.tm_sec = second;

		return tmRtp;
	}

	void set_length(WORD len)
	{
		m_length = htons(len);
	}

	WORD length()const
	{
		return ntohs(m_length);
	}

	DWORD GetNtpTime0()const
	{
		return ntohl(m_ntpTime0);
	}

	DWORD GetNtpTime1()const
	{
		return ntohl(m_ntpTime1);
	}

protected:

	WORD	m_extId;		//magic number,maygion:0x2008 onvif:0xABAC
	WORD	m_length;		//count of following DWORD,total len=4+length*sizeof(DWORD)
	DWORD	m_ntpTime0;		//used to match/show absolute osd time
	DWORD	m_ntpTime1;

#ifdef _CONFIG_BIG_ENDIAN
	BYTE	mbz : 5;		//must be zero
	BYTE	D : 1;
	BYTE	E : 1;
	BYTE	C : 1;
#else
	BYTE	C : 1;
	BYTE	E : 1;
	BYTE	D : 1;
	BYTE	mbz : 5;		//must be zero
#endif

	BYTE	m_CSeq;
	WORD	padding;

	DWORD	m_duration;	//current duration,used to show vod progress
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct tagVodProgressInfo
{
	tagVodProgressInfo()
	{
		Reset();
	}
	void Reset()
	{
		memset(this, 0, sizeof(*this));
	}
	DWORD		m_totalDuration;
	DWORD		m_curDuration;
	ULONGLONG	m_osdTime;		//为tagRTPExtension.m_ntpTime0(高32)和m_ntpTime1(低32)
};

struct JPEG_HEADER_S
{
#ifdef _CONFIG_BIG_ENDIAN
	unsigned int	off : 24;			/* fragment byte offset */
	unsigned int	tspec : 8;		/* type-specific field */
#else
	unsigned int	tspec : 8;		/* type-specific field */
	unsigned int	off : 24;			/* fragment byte offset */
#endif
	BYTE			type;           /* id of jpeg decoder params */
	BYTE			q;              /* quantization factor (or table id) */
	BYTE			width;          /* frame width in 8 pixel blocks */
	BYTE			height;         /* frame height in 8 pixel blocks */
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct JPEG_HEADER_QTABLE_S
{
	BYTE			mbz;
	BYTE			precision;
	unsigned short	length;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

enum eRtspError
{
#undef ITEM
#define ITEM(x)	x
	eRtspError_Unknown = -1,
	ITEM(eRtspError_Success),
	ITEM(eRtspError_InvalidParam),
	ITEM(eRtspError_ConnectFail),
	ITEM(eRtspError_ChannelDisconnect),
	ITEM(eRtspError_InvalidUserPassword),
	ITEM(eRtspError_InternalError),
	ITEM(eRtspError_NotFound),
#undef ITEM
};

enum eRtspReqType
{
	eRtspReq_Unknown = 0,
	eRtspReq_Options,
	eRtspReq_Describe,
	eRtspReq_Setup,
	eRtspReq_Play,
	eRtspReq_Teardown,
	eRtspReq_Get_Parameter,
	eRtspReq_Set_Parameter,
	eRtspReq_Announce,
	eRtspReq_Pause,
	//eRtspReq_CMD,
};

/*
enum
{
	PT_PCMU = 0,
	PT_G711A = 19,
	PT_G711U = 20,
	PT_AAC = 37,
	PT_H264 = 96,
};
*/
enum eRtspRequestCommand
{
	eRtsp_Unknown = 0,
	eRtsp_Options,
	eRtsp_Describe,
	eRtsp_Setup,
	eRtsp_Play,
	eRtsp_Teardown,
	eRtsp_Get_Parameter,
	eRtsp_Set_Parameter,
	eRtsp_Announce,
	eRtsp_Pause,
	eRtsp_CMD,
};

struct tagRtspServerConfig
{
	std::shared_ptr<Bear::Core::VirtualFolder>	mVirtualFolder;
	std::weak_ptr<AjaxCommandHandler>			mAjaxCommandHandler;
	std::shared_ptr<Bear::Core::UserMan>		mUserMan;
};

struct tagRtspClientConfig
{
	string mUserAgent;

};

}
}
}
}
