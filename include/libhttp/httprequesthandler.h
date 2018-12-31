#pragma once
#include "httpheaderinfo.h"
#include "httpackheader.h"
#include "auth/authman.h"
#include "base/iparam.h"
#include "libhttp/httpconfig.h"

namespace Bear {
namespace Core {
using namespace FileSystem;
namespace Net {
namespace Http {

class CHttpRequest;

//XiongWanPing 2013.06.09
//发块发送时，每块大小约4KB
class HTTP_EXPORT HttpRequestHandler :public UserInfo, public IParam
{
public:
	HttpRequestHandler();
	virtual ~HttpRequestHandler();

	void SetConfig(std::shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}

	void SetHttpRequest(CHttpRequest *httpRequest)
	{
		m_httpRequest = httpRequest;
	}

	virtual int Start(tagHttpHeaderInfo *headerInfo);
	virtual int Process();

	//用来支持大数据分块收发
	virtual bool IsSending()
	{
		return false;
	}

	virtual bool IsRecving()
	{
		return false;
	}

	virtual int Input(ByteBuffer& inbox)
	{
		ASSERT(IsRecving());
		UNREFERENCED_PARAMETER(inbox);

		ASSERT(FALSE);	//子类必须自己实现
		return 0;
	}

	void SetOutbox(ByteBuffer *outbox);

	eHttpHandlerStatus GetStatus()const
	{
		return m_status;
	}
	eUserLevel GetCurUserLevel()const
	{
		return m_headerInfo->m_curUserLevel;
	}

protected:
	//IParam#begin
	virtual std::string  GetString(std::string  name, std::string  defValue = "");
	virtual int GetInt(std::string  name, int defValue = 0);
	virtual bool IsParamExists(std::string  name);
	//IParam#end

	int	GetParamInt(const char *pszName, int nDefaultValue = 0);
	std::string  GetParamString(const char *pszName, const char *pszDefaultValue = "");
	BOOL IsParamOn(const char *pszName, BOOL bCheckOn = TRUE);
	BOOL IsParamExists(const char *pszName);

	eUserLevel GetUserLevel(std::string  user, std::string  password);
	bool IsAuthAction(const char *pszAction, const char *pszUserGroup = NULL);

	int Done()
	{
		SetStatus(eHttpHandlerStatus_Done);
		return 0;
	}
	void SetStatus(eHttpHandlerStatus status);
	virtual void OnStatusChanged(eHttpHandlerStatus oldStatus, eHttpHandlerStatus newStatus)
	{
		UNREFERENCED_PARAMETER(oldStatus);
		UNREFERENCED_PARAMETER(newStatus);
	}

	int Output(LPBYTE pData, int cbData);
	int Output(std::string  data)
	{
		return Output((LPBYTE)data.c_str(), (int)data.length());
	}

protected:
	CHttpRequest * m_httpRequest;
	tagHttpHeaderInfo		*m_headerInfo;
	ByteBuffer				*m_outbox;

	eHttpHandlerStatus		m_status;
	std::shared_ptr<tagWebServerConfig> mWebConfig;
};

}
}
}
}