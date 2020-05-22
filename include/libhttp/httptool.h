#pragma once
#include "base/namevalue.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {
//XiongWanPing 2013.06.08
class HTTP_EXPORT HttpTool
{
public:
	static int GetInt(std::string request, const char *name, int defaultValue = 0)
	{
		return GetInt(request.c_str(), name, defaultValue);
	}
	static int GetInt(const char *request, const char *name, int defaultValue = 0);
	static std::string  GetString(const char *request, const char *name, std::string  defaultValue = "");
	static void EscapeUrlString(char* psz);
	static std::string  GetUriExt(std::string  uri);
	static std::string  GetContentType(std::string  ext);
	static int ParseUrlParam(const char *pszUrl, std::string & uri, Core::NameValue& params);
	static int ParseUrlParam(std::string url, std::string & uri, Core::NameValue& params)
	{
		return ParseUrlParam(url.c_str(), uri, params);
	}
	static int ParseCookie(const char *pszCookie, Core::NameValue& cookies);
	static time_t ParseIfModifiedSince(const char *str);
	static time_t ParseIfModifiedSince(std::string sz)
	{
		return ParseIfModifiedSince(sz.c_str());
	}
	static int GetBasicAuthInfo(const char *request, std::string & usr, std::string & pwd);
	static BOOL IsSupportHttpMethod(const char *pszCmd);
	static BOOL IsSupportHttpMethod(std::string cmd)
	{
		return 	IsSupportHttpMethod(cmd.c_str());
	}
	static bool CheckFileCache(std::string  szFileName, time_t if_modified_since, std::string & szLastModifiedTime);

	static std::string  encodeURIComponent(std::string  value);
	static int ParseUrl(const std::string & url, std::string & host, int& port, std::string & pageUrl);

	static int Test();
	static std::string  Mid(const std::string & sz, const std::string & after, const std::string & before);
};
}
}
}
}