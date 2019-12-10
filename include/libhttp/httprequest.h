#pragma once

#include "httpheaderinfo.h"
#include "httpconfig.h"
#include "auth/userinfo.h"
#include "auth/authman.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
namespace Net {
namespace Http {

enum eHttpRequestStatus
{
	eHttpRequestStatus_Idle,
	eHttpRequestStatus_WaitHeader,	//等待完整的http header
	eHttpRequestStatus_Processing,
	eHttpRequestStatus_Error,
	eHttpRequestStatus_Done,		//所有ack都写入outbox后需要标记done,通知CHttpClient本次业务处理完成
};

//http socket转移,比如转交onvif,rtsp,asf模块处理
class IHttpRequestTransform
{
public:
	virtual ~IHttpRequestTransform() {}

	//target是目标模块,比如onvif/rtsp,asf
	//box中表示初始数据
	virtual int OnHttpRequestTransform(std::string  target, ByteBuffer& box) = 0;
};

class IHttpServerInfo
{
public:
	virtual ~IHttpServerInfo() {}
	virtual std::string  GetServerName() = 0;
};
#include "libhttp/httpconfig.h"
class HttpRequestHandler;
class HttpPostHandler;

//XiongWanPing 2013.06.08
//处理单个http事务
class HTTP_EXPORT HttpRequest :public UserInfo
{
public:
	HttpRequest();
	virtual ~HttpRequest();

	bool IsWebSocket()const
	{
		return mIsWebSocket;
	}

	void SetConfig(std::shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}

	string GetUrl()const
	{
		return m_headerInfo.mUrl;
	}

	void SetPeerAddr(std::string  peerAddr)
	{
		m_headerInfo.m_peerAddr = peerAddr;
	}

	void SetLocalAddr(std::string addr)
	{
		m_headerInfo.mLocalAddr = addr;
	}

	int Input(ByteBuffer& inbox);
	void SetOutbox(ByteBuffer *outbox);

	bool IsSending()const;
	virtual void Process();

	void SetHttpServerInfo(IHttpServerInfo *httpServerInfo)
	{
		m_httpServerInfo = httpServerInfo;
	}
	void SetTransformPtr(IHttpRequestTransform	*iHttpRequestTransform)
	{
		m_httpRequestTransform = iHttpRequestTransform;
	}

	void SetUserAuth(IUserInfo *userAuth)
	{
		m_userAuth = userAuth;
	}
	IUserInfo * GetUserAuth()
	{
		return m_userAuth;
	}

	//辅助实现分块返回,比如下载超大文件
	virtual bool IsDone();
	eHttpRequestStatus GetStatus()const
	{
		return m_eHttpRequestStatus;
	}

	int	GetParamInt(const char *pszName, int nDefaultValue = 0);
	std::string  GetParamString(const char *pszName, const char *pszDefaultValue = "");
	BOOL IsParamOn(const char *pszName, BOOL bCheckOn = TRUE);
	BOOL IsParamExists(const char *pszName);

	int OnRedirect(const char *pszPage);
	int Transform(std::string  target, ByteBuffer& box);

	bool IsAuthAction(const char *pszAction, const char *pszUserGroup = NULL);
protected:
	std::shared_ptr<HttpRequestHandler> CreateHandler(std::string  uri);

	void CheckSubmit();
	void SetStatus(eHttpRequestStatus status);

	void Reset();
	int Output(LPBYTE pData, int cbData);
	int Output(std::string  data)
	{
		return Output((LPBYTE)data.c_str(), (int)data.length());
	}

	int OnHeaderReady();
	int OnHeaderContentReady();
	int PreProcessHeader();
	int ParseHeader();

	int OnUnsupportedHttpMethod();

	int CheckAuth();
	BOOL IsNeedAuth();
	int CheckProcessPostRequest(ByteBuffer& inbox);

	eUserLevel GetCurUserLevel()const
	{
		return m_headerInfo.m_curUserLevel;
	}

	std::string  GetServerName();
protected:
	int CheckWebSocket();
	bool mIsWebSocket = false;
	ByteBuffer * m_outbox;
	ByteBuffer				m_outboxPending;		//有待发给m_outbox的数据
	//m_outboxPending给CHttpRequestHandlerXXX提供足够大的空间，保证至少能提交普通的http ack header+4KB数据

	eHttpRequestStatus		m_eHttpRequestStatus;

	tagHttpHeaderInfo		m_headerInfo;

	IUserInfo				*m_userAuth;
	IHttpServerInfo			*m_httpServerInfo;
	IHttpRequestTransform	*m_httpRequestTransform;

	std::shared_ptr<HttpRequestHandler> m_handler;
	static NameValue		*m_mapUriFile;//映射特殊的uri文件

	std::shared_ptr<tagWebServerConfig>	mWebConfig;
	std::shared_ptr<HttpPostHandler>		mHttpPostHandler;
};
}
}
}
}