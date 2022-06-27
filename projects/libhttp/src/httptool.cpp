#include "stdafx.h"
#include "httptool.h"
#include "libcrypt/base64ex.h"
#include <sys/stat.h>
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "HttpTool";

int HttpTool::GetInt(const char *request, const char *name, int defaultValue)
{
	string  value = GetString(request, name, "");
	if (value.empty())
		return defaultValue;
	return atoi(value.c_str());
}

string  HttpTool::GetString(const char *request, const char *name, string  defaultValue)
{
	string  value = defaultValue;

	string  key = StringTool::Format("\r\n%s: ", name);
	const char *psz = strstr(request, key.c_str());
	if (psz)
	{
		psz += key.length();
		const char *pszEnd = strstr(psz, "\r\n");
		if (pszEnd)
		{
			value = string(psz, pszEnd - psz);
		}
		else
		{
			value = psz;
		}
	}

	return value;
}

//把http url中的%xx转成实际数据
//比如IE会把空格转成%20,
//调用EscapeUrlString则把%20还原成空格
void HttpTool::EscapeUrlString(char* psz)
{
	if (!psz || !psz[0] || !strchr(psz, '%'))
	{
		return;
	}

	int dst = 0;
	int nc = (int)strlen(psz);
	for (int src = 0; src < nc; src++)
	{
		//char chh = psz[src];
		if (psz[src] == '%')
		{
			int ch;
			char *ps = psz + src + 1;
			int ret = sscanf(ps, "%02x", &ch);
			if (ret != 1)
			{
				LogW(TAG,"fail sscanf,ret=%d,sz=[%s]", ret,psz);
			}

			psz[dst++] = (BYTE)ch;
			src += 2;
		}
		else if (src != dst)
		{
			psz[dst++] = psz[src];
		}
		else
		{
			dst++;
		}
	}

	psz[dst] = 0;
}

//返回uri的后缀
//比如uri为/www/t.asp
//则GetUriExt返回.asp
string  HttpTool::GetUriExt(string  uri)
{
	int pos = (int)uri.rfind('.');
	if (pos <= 0)
	{
		return "";
	}

	string  ext = uri.substr(pos, uri.length() - pos);
	return ext;
}

string  HttpTool::GetContentType(string  ext)
{
	static const char *arr[] =
	{
		".htm",		"text/html",
		".html",	"text/html",
		".js",		"text/javascript",
		".ico",		"image/icon",
		".css",		"text/css",
		".png",		"image/png",
		".gif",		"image/gif",
		".jpg",		"image/jpeg",
		".jpeg",	"image/jpeg",
		".cfg",		"application/custom",
		".avi",		"video/mp4",//"video/x-msvideo";
		".mp4",		"video/mp4",
		".xlsx",	"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
		".xls",		"application/vnd.ms-excel",
		".pdf",		"application/pdf",
		".doc",		"application/msword",
		".docx",	"application/vnd.openxmlformats-officedocument.wordprocessingml.document",
		".ppt",		"application/vnd.ms-powerpoint",
		".pptx",	"application/vnd.openxmlformats-officedocument.presentationml.presentation",

	};

	for (int i = 0; i < COUNT_OF(arr); i += 2)
	{
		if (StringTool::CompareNoCase(ext, arr[i]) == 0)
		{
			return arr[i + 1];
		}
	}

	return "application/octet-stream";//binary like .rar,.ocx
}

//http://127.0.0.1/vs2.rar?usr=admin&pwd=admin&spaceTest%20_=123
//IE会把http cmd中的空格转成%20
//把/vs2.rar?usr=admin&pwd=admin解析成单独的item
int HttpTool::ParseUrlParam(const char *pszUrl, string & uri, NameValue& params)
{
	uri.clear();
	//params可能已存在其他参数，所以不要清空

	{
		char szUri[1024];
		CLR_BUF(szUri);

		char szFmt[64];
		_snprintf(szFmt, sizeof(szFmt) - 1, "%%%d[^?]?"
			, (int)sizeof(szUri) - 1
		);
		szFmt[sizeof(szFmt) - 1] = 0;
		int ret = sscanf(pszUrl, szFmt, szUri);
		if (ret < 1)
		{
			//nothing
		}

		EscapeUrlString(szUri);
		uri = szUri;
		StringTool::Replace(uri, "../", "");
	}

	const char *szParams = strchr(pszUrl, '?');
	if (!szParams)
	{
		return 0;
	}

	++szParams;

	//DT("uri=[%s]",uri.c_str());

	if (szParams[0] != 0)
	{
		//CArrayEx arrString;
		vector<string> arrString;
		StringTool::ExtractSubString(szParams, '&', arrString);
		//string ::CAutoDeleteString ad(arrString);
		int nc = (int)arrString.size();
		for (int i = 0; i < nc; i++)
		{
			auto& line = arrString[i];
			int pos = (int)line.find('=');
			if (pos != -1)
			{
				string  name = line.substr(0, pos);
				string  value = line.substr(pos+1);
				char szValue[4096];
				strncpy(szValue, value.c_str(), sizeof(szValue) - 1);
				EscapeUrlString(szValue);
				params.Set(name.c_str(), szValue);
			}
		}

	}

	return 0;
}

struct strlong
{
	const char* s;
	long l;
};

static int
strlong_compare(const void * v1, const void * v2)
{
	return strcmp(((struct strlong*) v1)->s, ((struct strlong*) v2)->s);
}

static int
strlong_search(char* str, struct strlong* tab, int n, long* lP)
{
	int i, h, l, r;

	l = 0;
	h = n - 1;
	while (1)
	{
		i = (h + l) / 2;
		r = strcmp(str, tab[i].s);
		if (r < 0)
			h = i - 1;
		else if (r > 0)
			l = i + 1;
		else
		{
			*lP = tab[i].l;
			return 1;
		}
		if (h < l)
			return 0;
	}
}

static void
pound_case(char* str)
{
	for (; *str != '\0'; ++str)
	{
		if (isupper((int)*str))
		{
			*str = (char)tolower((int)*str);
		}
	}
}

static int
scan_mon(char* str_mon, long* tm_monP)
{
	static struct strlong mon_tab[] = {
		{ "jan", 0 }, { "january", 0 },
		{ "feb", 1 }, { "february", 1 },
		{ "mar", 2 }, { "march", 2 },
		{ "apr", 3 }, { "april", 3 },
		{ "may", 4 },
		{ "jun", 5 }, { "june", 5 },
		{ "jul", 6 }, { "july", 6 },
		{ "aug", 7 }, { "august", 7 },
		{ "sep", 8 }, { "september", 8 },
		{ "oct", 9 }, { "october", 9 },
		{ "nov", 10 }, { "november", 10 },
		{ "dec", 11 }, { "december", 11 },
	};
	static int sorted = 0;

	if (!sorted)
	{
		qsort(
			mon_tab, sizeof(mon_tab) / sizeof(struct strlong),
			sizeof(struct strlong), strlong_compare);
		sorted = 1;
	}
	pound_case(str_mon);
	return strlong_search(
		str_mon, mon_tab, sizeof(mon_tab) / sizeof(struct strlong), tm_monP);
}
static int
scan_wday(char* str_wday, long* tm_wdayP)
{
	static struct strlong wday_tab[] = {
		{ "sun", 0 }, { "sunday", 0 },
		{ "mon", 1 }, { "monday", 1 },
		{ "tue", 2 }, { "tuesday", 2 },
		{ "wed", 3 }, { "wednesday", 3 },
		{ "thu", 4 }, { "thursday", 4 },
		{ "fri", 5 }, { "friday", 5 },
		{ "sat", 6 }, { "saturday", 6 },
	};
	static int sorted = 0;

	if (!sorted)
	{
		(void)qsort(
			wday_tab, sizeof(wday_tab) / sizeof(struct strlong),
			sizeof(struct strlong), strlong_compare);
		sorted = 1;
	}
	pound_case(str_wday);
	return strlong_search(
		str_wday, wday_tab, sizeof(wday_tab) / sizeof(struct strlong), tm_wdayP);
}

//If-Modified-Since: Mon, 08 Sep 2008 23:34:59 GMT;
time_t HttpTool::ParseIfModifiedSince(const char *str)
{
	struct tm tm;
	const char* cp;
	char str_mon[64], str_wday[64];
	int tm_sec, tm_min, tm_hour, tm_mday, tm_year;
	long tm_mon, tm_wday;
	time_t t;

	/* Initialize. */
	memset((char*)&tm, 0, sizeof(tm));

	/* Skip initial whitespace ourselves - sscanf is clumsy at this. */
	for (cp = str; *cp == ' ' || *cp == '\t'; ++cp)
	{

	}

	/* And do the sscanfs.  WARNING: you can add more formats here,
	** but be careful!  You can easily screw up the parsing of existing
	** formats when you add new ones.  The order is important.
	*/

	//DD-mth-YY HH:MM:SS GMT
	if (sscanf(cp, "%d-%[a-zA-Z]-%d %d:%d:%d GMT",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec) == 6 &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}

	//DD mth YY HH:MM:SS GMT 
	else if (sscanf(cp, "%d %[a-zA-Z] %d %d:%d:%d GMT",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec) == 6 &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}

	//HH:MM:SS GMT DD-mth-YY
	else if (sscanf(cp, "%d:%d:%d GMT %d-%[a-zA-Z]-%d",
		&tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon,
		&tm_year) == 6 &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}

	//HH:MM:SS GMT DD mth YY
	else if (sscanf(cp, "%d:%d:%d GMT %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon,
		&tm_year) == 6 &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}

	//wdy, DD-mth-YY HH:MM:SS GMT
	else if (sscanf(cp, "%[a-zA-Z], %d-%[a-zA-Z]-%d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec) == 7 &&
		scan_wday(str_wday, &tm_wday) &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}

	//wdy, DD mth YY HH:MM:SS GMT
	else if (sscanf(cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec) == 7 &&
		scan_wday(str_wday, &tm_wday) &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}

	//wdy mth DD HH:MM:SS GMT YY 
	else if (sscanf(cp, "%[a-zA-Z] %[a-zA-Z] %d %d:%d:%d GMT %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec,
		&tm_year) == 7 &&
		scan_wday(str_wday, &tm_wday) &&
		scan_mon(str_mon, &tm_mon))
	{
		tm.tm_wday = tm_wday;
		tm.tm_mon = tm_mon;
		tm.tm_mday = tm_mday;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_year = tm_year;
	}
	else
		return (time_t)-1;


	/*
	 {
		 DT("### cache time=%02d.%02d.%02d %02d:%02d:%02d",
			 tm.tm_year,tm.tm_mon+1,tm.tm_mday,
			 tm.tm_hour,tm.tm_min,tm.tm_sec);
	 }
	 //*/


	if (tm.tm_year > 1900)
		tm.tm_year -= 1900;
	else if (tm.tm_year < 70)
		tm.tm_year += 100;

	t = mktime(&tm);
	return t;
}

//注意pszCookie要以;结尾,否则最后一个cookie会漏掉
int HttpTool::ParseCookie(const char *pszCookie, NameValue& cookies)
{
	const char *ps = pszCookie;
	while (1)
	{
		char *pe = strstr((char*)ps, ";");
		if (!pe)
		{
			break;
		}

		char szLine[2048];
		CLR_BUF(szLine);
		int cp = MIN((int)sizeof(szLine) - 1, (long)(pe - (ps + 1)));
		if (cp <= 0)
		{
			break;
		}
		strncpy(szLine, ps + 1, cp);

		char szCookieName[32];
		char szCookieValue[1024];
		CLR_BUF(szCookieName);
		CLR_BUF(szCookieValue);
		char szFmt[64];


		//说明:
		//发现szFmt为"%%%d[^=]=%%%d[]"时不能解析const char *szLine="mediauuid=30306a4d-87f3-437f-ab93-cdb0135ac27a";
		//ret返回1,改用"%%%d[^=]=%%%ds"则能正常解析
		_snprintf(szFmt, sizeof(szFmt) - 1, "%%%d[^=]=%%%ds",
			(int)sizeof(szCookieName) - 1,
			(int)sizeof(szCookieValue) - 1
		);
		szFmt[sizeof(szFmt) - 1] = 0;
		szLine[sizeof(szLine) - 1] = 0;
		int ret = sscanf(szLine, szFmt, szCookieName, szCookieValue);
		if (ret == 2)
		{
			//cookiename不需要转化,cookievalue需要转化
			HttpTool::EscapeUrlString(szCookieValue);

			cookies.Set(szCookieName, szCookieValue);
		}

		ps = pe + 1;
	}

	return 0;
}

int HttpTool::GetBasicAuthInfo(const char *request, string & usr, string & pwd)
{
	const char *pszReq = request;
	const char *pszAuthKey = "Authorization: Basic ";
	const char *psz = StringTool::stristr(pszReq, pszAuthKey);
	if (psz)
	{
		const char *pszAuth = psz + strlen(pszAuthKey);
		const char *pszAuthEnd = strstr(pszAuth, "\r\n");
		if (pszAuthEnd && pszAuthEnd - pszAuth < 100)
		{
			char szUsrPwdEnc[120];
			memset(szUsrPwdEnc, 0, sizeof(szUsrPwdEnc));
			strncpy(szUsrPwdEnc, pszAuth, pszAuthEnd - pszAuth);
			string  szUsrPwd = Base64::Decode(szUsrPwdEnc);

			char szUsr[64];
			char szPwd[64];
			memset(szUsr, 0, sizeof(szUsr));
			memset(szPwd, 0, sizeof(szPwd));
			char szFmt[64];
			_snprintf(szFmt, sizeof(szFmt) - 1, "%%%d[^:]:%%%d[^:]",
				(int)sizeof(szUsr) - 1,
				(int)sizeof(szPwd) - 1
			);
			szFmt[sizeof(szFmt) - 1] = 0;

			int ret = sscanf(szUsrPwd.c_str(), szFmt, szUsr, szPwd);
			//DT("sscanf ret=%d",ret);
			if (ret >= 1)//密码为空时,ret为1
			{
				usr = szUsr;
				pwd = szPwd;
			}
		}
	}

	return 0;
}

//目前只支持POST和GET
BOOL HttpTool::IsSupportHttpMethod(const char *pszCmd)
{
	if (!pszCmd || pszCmd[0] == 0)
		return FALSE;
	if (strcmp(pszCmd, "GET") == 0 || strcmp(pszCmd, "POST") == 0)
	{
		return TRUE;
	}

	return FALSE;
}

//返回true表示可以用cache文件
bool HttpTool::CheckFileCache(string  szFileName, time_t if_modified_since, string & szLastModifiedTime)
{
	szLastModifiedTime.clear();
	bool bCanUseCacheFile = false;

	struct _stat fs;
	memset(&fs, 0, sizeof(fs));
	int ret = _stat(szFileName.c_str(), &fs);
	if (ret == 0)
	{
		//m_if_modified_since保存的是UTC(GMT)时间
		//而stat得到的是本地时间，需要进行转换后再比较
		//int delta = abs(m_if_modified_since - fs.st_mtime);
		{
			struct tm *ptm = NULL;
#ifdef _CONFIG_ANDROID
			time_t tt = (time_t)fs.st_mtime;
			ptm = gmtime(&tt);
#else
			ptm = gmtime(&fs.st_mtime);
#endif
			if (ptm)
			{
				struct tm tm = *ptm;
				if (tm.tm_year > 1900)
					tm.tm_year -= 1900;
				else if (tm.tm_year < 70)
					tm.tm_year += 100;

				time_t t = mktime(&tm);
				bCanUseCacheFile = (if_modified_since == t);
			}

		}

		{
			char szBuf[128];
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#ifdef _CONFIG_ANDROID
			time_t tt = (time_t)fs.st_mtime;
			strftime(szBuf, sizeof(szBuf), RFC1123FMT, gmtime(&tt));
#else
			strftime(szBuf, sizeof(szBuf), RFC1123FMT, gmtime(&fs.st_mtime));
#endif
			//_snprintf( szLastModifiedTime,sizeof(szLastModifiedTime)-1,
			//	"Last-Modified: %s\r\n", 
			//	szBuf
			//	);
			szLastModifiedTime = szBuf;
		}
	}

	return bCanUseCacheFile;
}

//XiongWanPing 2013.08.05
//测试方法:把c++测试结果和w3schools的网页结果比较
/*
http://www.w3schools.com/jsref/tryit.asp?filename=tryjsref_encodeuricomponent
<script>
var uri;

uri="`1234567890-=";
document.write(encodeURIComponent(uri));
document.write("<br>");

uri="~!@#$%^&*()_+";
document.write(encodeURIComponent(uri));
document.write("<br>");


uri="{}:\"|<>?";
document.write(encodeURIComponent(uri));
document.write("<br>");


uri="[];\'\\,./";
document.write(encodeURIComponent(uri));
document.write("<br>");

</script>

//*/
string  HttpTool::encodeURIComponent(string  value)
{
	if (value.empty())
	{
		return value;
	}

	string  encode;

	static char arr[] = "-~!*()_\'.";
	//不需要编码的为0~9,a~z,A~Z和arr中的char
	const int len = (int)value.length();
	const char *ps = value.c_str();
	for (int i = 0; i < len; i++)
	{
		char ch = *ps++;
		bool number = (ch >= '0' && ch <= '9');
		bool alpha = (ch >= 'a' && ch <= 'z') | (ch >= 'A' && ch <= 'Z');
		bool directChar = (strchr(arr, ch) != NULL);

		if (number || alpha || directChar)
		{
			StringTool::AppendFormat(encode, "%c", ch);
		}
		else
		{
			StringTool::AppendFormat(encode, "%%%hhX", ch);
		}
	}

	return encode;
}

int HttpTool::Test()
{
#ifdef _DEBUG
	string  encode;

	encode = encodeURIComponent("`1234567890-=");
	DT("%s", encode.c_str());

	encode = encodeURIComponent("~!@#$%^&*()_+");
	DT("%s", encode.c_str());

	encode = encodeURIComponent("{}:\"|<>?");
	DT("%s", encode.c_str());

	encode = encodeURIComponent("[];\'\\,./");
	DT("%s", encode.c_str());

#endif
	return 0;
}

bool HttpTool::isHttps(const std::string& url)
{
	auto p = StringTool::stristr(url.c_str(), "https://");
	return p == url.c_str();
}

//从完整的url中解析出host,port和pageUrl
//host可能为dns或ip
//pageUrl一般以/开头
int HttpTool::ParseUrl(const string & url, string & szHost, int& port, string & pageUrl, bool* useHttps)
{
	string  szUrl(url);
	szHost = "";
	port = 80;
	pageUrl = "";
	if (useHttps)
	{
		*useHttps = false;
	}

	char host[256];
	char page[4 * 1024];
	string  fmt;
	memset(host, 0, sizeof(host));
	memset(page, 0, sizeof(page));

	bool enableHttps = false;
	//去掉可选的http:前缀和https:前缀
	const char *p = StringTool::stristr(szUrl.c_str(), "http://");
	if (p == szUrl.c_str())
	{
		szUrl = StringTool::Right(url, (int)(url.length() - strlen("http://")));
	}
	else
	{
		p=StringTool::stristr(szUrl.c_str(), "https://");
		if (p == szUrl.c_str())
		{
			enableHttps = true;
			szUrl = StringTool::Right(url, (int)(url.length() - strlen("https://")));
		}
	}

	fmt = StringTool::Format("%%%d[^/]%%%ds", sizeof(host) - 1, sizeof(page) - 1);

	int ret = sscanf(szUrl.c_str(), fmt.c_str(), host, page);
	if (ret < 1)
	{
		return -1;
	}

	pageUrl = page;
	//当url中有空格时这里提取到的page可能不完整
	if (!pageUrl.empty())
	{
		auto pos = szUrl.find(page);
		pageUrl = StringTool::Right(szUrl, (long)(szUrl.length() - pos));
	}

	port = enableHttps?443:80;
	//提取端口
	char *psz = strchr(host, ':');
	if (psz)
	{
		*psz = 0;
		szHost = host;
		int nPort = atoi(psz + 1);
		if (nPort > 0)
		{
			port = nPort;
		}
	}
	else
	{
		szHost = host;
	}


	if (useHttps)
	{
		*useHttps = enableHttps;
	}

	if (host[0] == '\0')
	{
		return -1;
	}

	return 0;
}

//在sz中提取after与before之间的字符串
string  HttpTool::Mid(const string & sz, const string & after, const string & before)
{
	auto pos = sz.find(after);
	if (pos != -1)
	{
		pos += after.length();
		auto posEnd = sz.find(before, pos);
		if (posEnd != -1)
		{
			return sz.substr(pos, posEnd - pos);
		}
	}

	return "";
}

}
}
}
}