#pragma once

#include "httpheaderinfo.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {
//XiongWanPing 2013.06.20
//用来统一管理http ack header
class HTTP_EXPORT CHttpAckHeader
{
public:
	CHttpAckHeader();
	virtual ~CHttpAckHeader();

	void SetHttpVersion(std::string  httpVersion)
	{
		ASSERT(httpVersion == "1.0" || httpVersion == "1.1");

		m_httpVersion = httpVersion;
	}

	void SetStatusCode(std::string  statusCode)
	{
		ASSERT(!statusCode.empty());

		m_statusCode = statusCode;
	}

	void SetContentType(std::string  contentType)
	{
		m_contentType = contentType;
	}

	void SetContentLength(size_t len)
	{
		std::string  szLen;
		Core::StringTool::AppendFormat(szLen, "%lu", len);
		SetField("Content-Length", szLen);
	}

	void SetConnection(std::string  connection)
	{
		ASSERT(connection == "close" || connection == "Keep-Alive");
		m_connection = connection;
	}

	static void SetServerDesc(std::string  serverDesc)
	{
		m_serverDesc = serverDesc;
	}

	void SetCacheControl(std::string  cacheControl)
	{
		SetField("Cache-Control", cacheControl);
	}
	void SetContentRange(std::string  contentRange)
	{
		SetField("Content-Range", contentRange);
	}

	void SetContentDisposition(std::string  value)
	{
		SetField("Content-disposition", value);
	}

	void SetExpires(std::string  Expires)
	{
		SetField("Expires", Expires);
	}

	//应优先采用专门的SetXXX(可做有效性验证)
	void SetField(std::string  name, std::string  value)
	{
		m_fields.Set(name.c_str(), value.c_str());
	}

	std::string  ack(BOOL addTail = TRUE);//返回ack header
protected:
	static std::string 	m_serverDesc;	//

	//必须存在的field
	std::string 	m_httpVersion;	//1.1 or 1.0,默认为1.0
	std::string 	m_statusCode;	//默认为200 OK
	std::string 	m_contentType;	//text/xml;charset=UTF-8
	std::string 	m_connection;	//close or Keep-Alive(默认)

	//可选的field:
	//Content-Length
	Core::NameValue	m_fields;
};

}
}
}
}