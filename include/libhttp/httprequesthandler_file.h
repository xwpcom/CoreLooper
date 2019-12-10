#pragma once
#include "httprequesthandler.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2013.06.09
//处理普通文件
class HTTP_EXPORT HttpRequestHandler_File :public HttpRequestHandler
{
public:
	HttpRequestHandler_File();
	virtual ~HttpRequestHandler_File();

	virtual int Start(tagHttpHeaderInfo *headerInfo);
	virtual int Process();
	virtual bool IsSending();

	static void AddUriFileMap(std::string  uri, std::string  filepath);
	static void EmptyUriFileMap();

protected:
	int OnFileNoFound(std::string  uri);
	int OnFileNoAuth();
	int OutputPlainText(std::string  text, std::string  statusCode = "200 OK");
	bool IsPartialContent()
	{
		return m_headerInfo->m_range != 0;
	}

protected:

	FILE * m_hFile;
	size_t	m_fileSize;
	size_t	m_fileOffset;

	static NameValue	*m_mapUriFile;//映射特殊的uri文件
};
}
}
}
}