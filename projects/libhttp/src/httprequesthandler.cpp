#include "stdafx.h"
#include "httprequesthandler.h"
#include "file/virtualfolder.h"
#include "httprequest.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpRequestHandler::HttpRequestHandler()
{
	m_httpRequest = NULL;
	m_headerInfo = NULL;
	m_outbox = NULL;
	m_status = eHttpHandlerStatus_Idle;
}

HttpRequestHandler::~HttpRequestHandler()
{
}

void HttpRequestHandler::SetStatus(eHttpHandlerStatus status)
{
	eHttpHandlerStatus statusSave = m_status;
	if (status != m_status)
	{
		m_status = status;

		OnStatusChanged(statusSave, m_status);
	}
}


int HttpRequestHandler::Process()
{
	return 0;
}

int HttpRequestHandler::Start(tagHttpHeaderInfo *headerInfo)
{
	ASSERT(!m_headerInfo);
	m_headerInfo = headerInfo;
	SetUserLevel(headerInfo->m_curUserLevel);

	return 0;
}

void HttpRequestHandler::SetOutbox(ByteBuffer *outbox)
{
	m_outbox = outbox;
}

int HttpRequestHandler::Output(LPBYTE pData, int cbData)
{
	if (!m_outbox)
	{
		DW("fail Output,m_outbox is NULL");
		ASSERT(FALSE);
		return -1;
	}

	int ret = m_outbox->Write(pData, cbData);
	if (ret != cbData)
	{
		DW("fail Output,cbData=%d,ret=%d", cbData, ret);
		ASSERT(FALSE);
	}

	return ret;
}

int	HttpRequestHandler::GetParamInt(const char *pszName, int nDefaultValue)
{
	return m_httpRequest->GetParamInt(pszName, nDefaultValue);
}

string  HttpRequestHandler::GetParamString(const char *pszName, const char *defValue)
{
	return m_httpRequest->GetParamString(pszName, defValue);
}

BOOL HttpRequestHandler::IsParamExists(const char *name)
{
	return m_httpRequest->IsParamExists(name) != 0;
}
//返回pszName值是否为on,一般用于checkbox
BOOL HttpRequestHandler::IsParamOn(const char *pszName, BOOL bCheckOn)
{
	return m_httpRequest->IsParamOn(pszName, bCheckOn);
}

eUserLevel HttpRequestHandler::GetUserLevel(string  user, string  password)
{
	IUserInfo * userAuth = m_httpRequest->GetUserAuth();
	if (userAuth)
	{
		return userAuth->GetUserLevel(user, password);
	}

	return eUser_Invalid;
}

bool HttpRequestHandler::IsAuthAction(const char *pszAction, const char *pszUserGroup)
{
	return m_httpRequest->IsAuthAction(pszAction, pszUserGroup);
}

//IParam#begin
string  HttpRequestHandler::GetString(string  name, string  defValue)
{
	return m_httpRequest->GetParamString(name.c_str(), defValue.c_str());
}

int HttpRequestHandler::GetInt(string  name, int defValue)
{
	return m_httpRequest->GetParamInt(name.c_str(), defValue);
}

bool HttpRequestHandler::IsParamExists(string  name)
{
	return m_httpRequest->IsParamExists(name.c_str()) != 0;
}
//IParam#end

}
}
}
}