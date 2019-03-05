#include "stdafx.h"
#include "httprequest.h"
#include "httptool.h"
#include "httppostparser.h"
#include "httprequesthandler.h"
#include "httprequesthandler_ajax.h"
#include "httprequesthandler_cgi.h"
#include "httprequesthandler_file.h"
#include "HttpPostHandler.h"

using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

NameValue *CHttpRequest::m_mapUriFile = NULL;

CHttpRequest::CHttpRequest()
{
	m_outbox = NULL;
	m_httpRequestTransform = NULL;
	m_httpServerInfo = NULL;
	m_userAuth = this;

	Reset();
}

CHttpRequest::~CHttpRequest()
{
	//DW("CHttpRequest::~CHttpRequest,this=0x%x",this);
	m_handler = nullptr;
}

void CHttpRequest::Reset()
{
	m_eHttpRequestStatus = eHttpRequestStatus_Idle;
	m_headerInfo.Reset();

	ASSERT(!m_handler);
}

//由于需要支持超大文件上传、下载，所以需要由CHttpRequest来负责解析完整的request body
//并且需要渐进处理机制
int CHttpRequest::Input(ByteBuffer& inbox)
{
	if (mHttpPostHandler)
	{
		int ret = mHttpPostHandler->Input(inbox);
		if (ret)
		{
			SetStatus(eHttpRequestStatus_Error);
			return -1;
		}

		//处理http post时要返回一些数据，否则浏览器可能一直等待
		if (mHttpPostHandler->IsDone())
		{
			/*
			Output(
				"HTTP/1.1 302\r\n"
				"Content-Type: text/html\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"0"
				"\r\n"
				"\r\n"
			);
			*/
			Output(
				"HTTP/1.1 200 OK\r\n"
				"\r\n"
			);

			SetStatus(eHttpRequestStatus_Done);
		}
		return 0;
	}

	if (m_handler)
	{
		//说明已收到一个完整的http request header,并且正在进行处理

		if (m_handler->IsRecving())
		{
			//handler支持接收大数据
			return m_handler->Input(inbox);
		}

		//当前handler完成后才处理
		//DW("delay parse http request");
		return 0;
	}

	//注意:为方便http 文本解析,上层已确保inbox始终以'\0'结尾
	LPBYTE pData = inbox.GetDataPointer();
	int cbData = inbox.GetActualDataLength();

	if (cbData <= 4)//\r\n\r\n
	{
		return 0;
	}

	//ASSERT(pData[0]!='\r' && pData[0]!='\n');//提前检测解析错误

	if (pData[0] == '\r' && pData[1] == '\n')
	{
		DW("eat bad pending data for IE6");
		inbox.Eat(2);
		pData = inbox.GetDataPointer();
		cbData -= 2;
		if (cbData <= 4)//\r\n\r\n
		{
			return 0;
		}
	}

	const char *szReqBuf = (char*)pData;
	//即使是超大文件，它的http header也不会很大,只是http body会比较大

	//判断浏览器发送完http request的算法:
	//先检查是否收到了\r\n\r\n,如果没有收到,则需要再次recv
	//如果收到了\r\n\r\n,则检查是否存在Content-Length
	//如果不存在Content-Length,则表示已收到完整的http cmd,可以开始处理
	//如果包含Content-Length,则在\r\n\r\n后面收到Content-Length个字节后表示recv http request完成
	BOOL headerContentOK = FALSE;
	tagHttpHeaderInfo& hi = m_headerInfo;
	if (hi.m_headerLength)
	{
		//m_nHttpHeaderLength不为0表示已收到\r\n\r\n
		//ASSERT(hi.m_contentLength);
		if (cbData >= hi.m_headerLength + hi.m_contentLength)
		{
			headerContentOK = TRUE;
		}
	}
	else
	{
		//检查\r\n\r\n
		const char *pszHeader = strstr(szReqBuf, "\r\n\r\n");
		if (pszHeader != 0)
		{
			hi.m_headerLength = (int)(pszHeader - szReqBuf + 4);//4 is "\r\n\r\n";

			//检查是否存在Content-Length
			ASSERT(!hi.m_contentLength);
			hi.m_contentLength = HttpTool::GetInt(szReqBuf, "Content-Length");
			inbox.MakeSureEndWithNull();
			m_headerInfo.m_request = (char*)pData;
			m_headerInfo.m_len = hi.m_headerLength;//如果有content,后面会再修改

			{
				//传感器2G网络，有时多发几个0,可能是起保活作用，这里屏蔽掉
				auto ps = m_headerInfo.m_request;
				if (strncmp(ps, "0GET", 4) == 0 || strncmp(ps, "?GET", 4) == 0)
				{
					int bytes = 1;
					m_headerInfo.m_request -= bytes;
					m_headerInfo.m_len -= bytes;
					inbox.Eat(bytes);
				}
				else if (strncmp(ps, "00GET", 5) == 0)
				{
					int bytes = 2;
					m_headerInfo.m_request += bytes;
					m_headerInfo.m_len -= bytes;
					inbox.Eat(bytes);
				}
			}

			int ret = OnHeaderReady();
#ifdef _MINI_HTTP
			if (ret)
			{
				static int idx = -1;
				++idx;
				DG("httpFail(%s)[%04d],drop data[%s]", m_headerInfo.m_peerAddr.c_str(), idx, m_headerInfo.m_request);
				inbox.clear();
				m_headerInfo.Reset();
				return -1;
			}
			else
			{
				if (strstr(m_headerInfo.m_request, "McuDeviceStatus.xml") == nullptr)
				{
					static int idx = -1;
					++idx;
					//DV("httpOK(%s)[%04d],data[%s]", m_headerInfo.m_peerAddr.c_str(), idx, m_headerInfo.m_request);
				}
			}
#endif

			if (m_headerInfo.m_httpMethod == "POST")
			{
				int ret = CheckProcessPostRequest(inbox);
				if (ret == 0)
				{
					return 0;
				}
			}

			if (cbData >= hi.m_headerLength + hi.m_contentLength)
			{
				headerContentOK = TRUE;
			}

			//这里可以限制m_nContentLength的大小，并做特殊处理
			//如果m_nContentLength为特大，并且不是POST升级firmware,则可以在这里提前close
			//如果是POST升级firmware,则需要边read边执行升级操作
			//firmware可能有几MB，不可能全部读完后才开始升级.
		}
	}

	if (m_headerInfo.m_httpMethod == "POST")
	{
		int ret = CheckProcessPostRequest(inbox);
		if (ret == 0)
		{
			return 0;
		}
	}

	if (headerContentOK)
	{
		//已收到完整http cmd
		//IE6.0有时在最后面会多加\r\n

		const int len = hi.m_headerLength + hi.m_contentLength;
		BYTE chSave = pData[len];
		pData[len] = 0;
		m_headerInfo.m_request = (char*)pData;
		m_headerInfo.m_len = len;
		OnHeaderContentReady();
		pData[len] = chSave;
		inbox.Eat(len);

		return 0;
	}


	return 0;
}

void CHttpRequest::SetOutbox(ByteBuffer *outbox)
{
	m_outbox = outbox;
}

//解析到完整的http header(不包括contentLength指定的数据)之后会调用本接口
int CHttpRequest::OnHeaderReady()
{
	int ret = ParseHeader();
	return ret;
}

//解析到完整的http header+content之后会调用本接口
int CHttpRequest::OnHeaderContentReady()
{
	if (m_handler)
	{
		ASSERT(FALSE);
		return -1;
	}

	char *request = m_headerInfo.m_request;
	int len = m_headerInfo.m_len;
	ASSERT(request[len] == '\0');

#ifdef _MSC_VER_DEBUG
	{
		static int idx = 0;
		string  filepath;
#ifdef _DEBUG
		filepath = StringTool::Format("g:/test/device/%04d_req.bin", idx++);
#else
		filepath.Format("%s/http/%04d_req.bin", ShellTool::GetAppPath().c_str(), idx++);
#endif

		File::Dump(request, len, filepath.c_str());
	}
#endif

	int ret = PreProcessHeader();
	if (ret == 0)
	{
		return 0;
	}

	m_outboxPending.PrepareBuf(16 * 1024);

	if (!HttpTool::IsSupportHttpMethod(m_headerInfo.m_httpMethod))
	{
		OnUnsupportedHttpMethod();
		return 0;
	}

	string & uri = m_headerInfo.m_uri;
	if (uri == "/" || uri.empty())
	{
		if (CheckAuth() == 0)
			uri = "index.html";
		else
			uri = "login.html";
	}

	if (IsNeedAuth() && CheckAuth() != 0)
	{
		OnRedirect("login.html");
		return 0;
	}

	ASSERT(!m_handler);
	m_handler = CreateHandler(m_headerInfo.m_uri);
	if (!m_handler)
	{
		DW("no handler for [%s]", m_headerInfo.m_uri.c_str());
		ASSERT(FALSE);
		return 0;
	}

	m_handler->SetHttpRequest(this);
	m_handler->SetOutbox(&m_outboxPending);
	m_handler->SetUserPassword(GetUserName(), GetPassword());
	m_handler->Start(&m_headerInfo);

	return 0;
}

int CHttpRequest::Transform(string  target, ByteBuffer& box)
{
#ifdef _MSC_VER_DEBUG
	File::Dump(box, "g:/test/http.post.bin");
#endif

	int ret = -1;
	if (m_httpRequestTransform)
	{
		ret = m_httpRequestTransform->OnHttpRequestTransform(target, box);
	}
	else if (target == "post")
	{
		ASSERT(!mHttpPostHandler);
		mHttpPostHandler = make_shared<HttpPostHandler>();
		mHttpPostHandler->SetConfig(mWebConfig);
		int ret = mHttpPostHandler->Input(box);
		if (ret)
		{
			string  ack = StringTool::Format("HTTP/1.1 %d Error\r\n\r\n", ret);
			Output(ack);
			SetStatus(eHttpRequestStatus_Error);
			return -1;
		}

		if (mHttpPostHandler->IsDone())
		{
			/*
			Output(
				"HTTP/1.1 302\r\n"
				"Content-Type: text/html\r\n"
				"Transfer-Encoding: chunked\r\n"
				"\r\n"
				"0"
				"\r\n"
				"\r\n"
			);
			*/
			Output(
				"HTTP/1.1 200 OK\r\n"
				"\r\n"
			);

			SetStatus(eHttpRequestStatus_Done);
		}
		return 0;
	}
	else
	{
		DW("no transform ptr for [%s]", target.c_str());
		ASSERT(FALSE);
	}

	SetStatus(eHttpRequestStatus_Done);
	return ret;
}

//parse http header
int CHttpRequest::ParseHeader()
{
	char *request = m_headerInfo.m_request;
	UINT len = m_headerInfo.m_len;
	//ASSERT(request[len]=='\0');

	m_headerInfo.m_curUserLevel = eUser_Invalid;

	int ret = 0;

	{
		char szReq[128];
		char szUrl[4 * 1024];//url可以用?加参数,所以可能比较长
		char szHttpVer[16];
		CLR_BUF(szReq);
		CLR_BUF(szUrl);
		CLR_BUF(szHttpVer);

		{
			char szFmt[128];
			CLR_BUF(szFmt);
			_snprintf(szFmt, sizeof(szFmt) - 1, "%%%u[^ ] %%%u[^ ] %%%u[^ \r]",
				(long)sizeof(szReq) - 1,
				(long)sizeof(szUrl) - 1,
				(long)sizeof(szHttpVer) - 1
			);
			szFmt[sizeof(szFmt) - 1] = 0;
			ret = sscanf(request, szFmt, szReq, szUrl, szHttpVer);
			if (ret != 3)
			{
				DW("invalid http request");
				return -1;
			}
		}

		if (!HttpTool::IsSupportHttpMethod(szReq))
		{
			return -1;
		}

		m_headerInfo.m_httpMethod = szReq;
		HttpTool::EscapeUrlString(szUrl);
		m_headerInfo.mUrl = szUrl;
		//DW("HTTP Method=[%s]",m_headerInfo.m_httpMethod.c_str());

		HttpTool::ParseUrlParam(szUrl, m_headerInfo.m_uri, m_headerInfo.m_urlParam);

		{
			m_headerInfo.m_szReferer = HttpTool::GetString(request, "Referer");
			//referer中可能带有参数,解析方式同url,但不要更新uri
			//XiongWanPing 2013.02.27不需要解析Referer中的参数
		}

		{
			const char *pszKey = "\r\nRange:";
			const char *psz = strstr(request, pszKey);
			if (psz)
			{
				pszKey = "bytes=";
				psz = strstr(request, pszKey);
				if (psz)
				{
					psz += strlen(pszKey);

					m_headerInfo.m_range = 0;
					m_headerInfo.m_rangeEnd = 0;//m_rangeEnd最大为fileSize-1
					DWORD range = 0;
					DWORD rangeEnd = 0;
					ret = sscanf(psz, "%u-%u", &range, &rangeEnd);

					if (ret <= 0)
					{
						DW("fail get m_range");
					}

					m_headerInfo.m_range = range;
					m_headerInfo.m_rangeEnd=rangeEnd;

					if (m_headerInfo.m_rangeEnd < 0 || m_headerInfo.m_rangeEnd < m_headerInfo.m_range)
					{
						m_headerInfo.m_rangeEnd = m_headerInfo.m_range;
					}
					//DW("m_range=%d",m_range);
				}
			}
		}

		{
			string  szTime = HttpTool::GetString(request, "If-Modified-Since");
			if (!szTime.empty())
			{
				m_headerInfo.m_if_modified_since = HttpTool::ParseIfModifiedSince(szTime);
			}
		}

		{
			const char *pszKey = "\r\nCookie:";
			char *pszCookie = strstr(request, pszKey);
			if (pszCookie)
			{
				char *pszCookieEnd = strstr(pszCookie + strlen(pszKey), "\r\n");
				if (pszCookieEnd)
				{
					char chSave0 = pszCookieEnd[0];
					char chSave1 = pszCookieEnd[1];
					pszCookieEnd[0] = ';';
					pszCookieEnd[1] = 0;
					HttpTool::ParseCookie(pszCookie + strlen(pszKey), m_headerInfo.m_cookies);
					pszCookieEnd[0] = chSave0;
					pszCookieEnd[1] = chSave1;

				}
			}
		}

		{
			//当采用POST上传文件时,会有如下数据,需要把boundary提取出来
			//Content-Type: multipart/form-data; boundary=---------------------------7da20535d00fa\r\n
			const char *pszKey = "\r\nContent-Type: multipart/form-data; boundary=";
			char *ps = strstr(request, pszKey);
			if (ps)
			{
				char *pe = strstr(ps + strlen(pszKey), "\r\n");
				if (pe)
				{
					//把boundary当做一个cookie来添加
					char chSave = *pe;
					*pe = 0;//截断
					const char *value = ps + strlen(pszKey);
					*pe = chSave;//恢复
					m_headerInfo.m_cookies.Set("boundary", value);
				}
			}
		}

		//当升级文件或者上传较大文件时,request比较小,不能接收完整的表单数据,此时不能解析
		BOOL bRecvFormDataFinished = (len < (sizeof(request) - 1));
		if (m_headerInfo.m_contentLength > 0 && bRecvFormDataFinished)
		{
			//解析表单数据
			const int pszLen = m_headerInfo.m_contentLength + 2 + 2;
			char *psz = new char[pszLen];
			ASSERT(psz);
			if (psz)
			{
				strncpy(psz, request + m_headerInfo.m_headerLength, m_headerInfo.m_contentLength + 1);
				psz[pszLen - 1] = 0;
				char *pszEnd = psz + strlen(psz);

				//表单数据和url参数格式是一样的,处理方法相同
				{
					char *pszParams = psz;
					while (pszParams && pszParams[0] && pszParams < pszEnd)
					{
						char szName[256];
						char szValue[2048];
						CLR_BUF(szName);
						CLR_BUF(szValue);

						char szFmt[64];
						_snprintf(szFmt, sizeof(szFmt) - 1, "%%%u[^=]=%%%u[^&]&",
							(long)sizeof(szName) - 1,
							(long)sizeof(szValue) - 1
						);
						ret = sscanf(pszParams, szFmt, szName, szValue);
						if (ret == 0)
						{
							break;
						}
						HttpTool::EscapeUrlString(szValue);
						m_headerInfo.m_urlParam.Set(szName, szValue);

						pszParams = strchr(pszParams, '&');
						if (pszParams)
							pszParams++;

					}
				}

				delete[]psz;
				psz = NULL;
			}
		}
	}

	//check user level
	{
		//根据http参数和vs设置来确定当前用户级别
		string  usr = GetParamString("user");
		string  pwd = GetParamString("password");
		//当usr和user都不存在时，使用http auth basic
		if (usr.empty() && !IsParamExists("user"))
		{
			HttpTool::GetBasicAuthInfo(request, usr, pwd);
		}

		if (m_userAuth)
		{
			m_userAuth->SetUserPassword(usr, pwd);
			m_headerInfo.m_curUserLevel = m_userAuth->GetUserLevel(usr, pwd);
		}
	}

	return 0;
}

int	CHttpRequest::GetParamInt(const char *pszName, int nDefaultValue)
{
	return m_headerInfo.m_urlParam.GetInt(pszName, nDefaultValue);
}

//返回pszName值是否为on,一般用于checkbox
BOOL CHttpRequest::IsParamOn(const char *pszName, BOOL bCheckOn)
{
	string  szValue = GetParamString(pszName);
	if (szValue.empty())
		return bCheckOn;
	return StringTool::CompareNoCase(szValue, "on") == 0;
}

//判断参数是否存在
BOOL CHttpRequest::IsParamExists(const char *pszName)
{
	//url优先
	if (m_headerInfo.m_urlParam.IsExists(pszName))
	{
		return true;
	}

	//然后才是cookie
	if (m_headerInfo.m_cookies.IsExists(pszName))
	{
		return true;
	}

	return FALSE;
}

//在url param,cookie,表单中查找pszName字段
string  CHttpRequest::GetParamString(const char *pszName, const char *pszDefaultValue)
{
	string  value = m_headerInfo.m_urlParam.GetString(pszName, pszDefaultValue);
	if (StringTool::CompareNoCase(value, pszDefaultValue) == 0)
	{
		value = m_headerInfo.m_cookies.GetString(pszName, pszDefaultValue);
	}

	return value;
}

void CHttpRequest::CheckSubmit()
{
	ASSERT(m_outbox);

	//把m_outboxPending数据提交到m_outbox
	if (!m_outboxPending.empty() && m_outbox)
	{
		LPBYTE src = m_outboxPending.GetDataPointer();
		int len = m_outboxPending.GetActualDataLength();
		int free = m_outbox->GetFreeSize();
		int eat = MIN(len, free);
		if (eat > 0)
		{
			int ret = m_outbox->Write(src, eat);
			if (ret > 0)
			{
				m_outboxPending.Eat(ret);
			}
		}
	}
}

void CHttpRequest::Process()
{
	CheckSubmit();

	if (m_handler)
	{
		eHttpHandlerStatus status = m_handler->GetStatus();
		switch (status)
		{
		case eHttpHandlerStatus_Processing:
		{
			m_handler->Process();
			break;
		}
		default:
		{
			m_handler = nullptr;

			SetStatus(eHttpRequestStatus_Done);
			break;
		}
		}
	}
}

int CHttpRequest::Output(LPBYTE pData, int cbData)
{
	ASSERT(m_outbox);

	int ret = m_outboxPending.Write(pData, cbData);
	if (ret != cbData)
	{
		DW("CHttpRequest outbox overflow");
		ASSERT(FALSE);
		return -1;
	}

	CheckSubmit();
	return 0;
}

string  CHttpRequest::GetServerName()
{
	string  name;
	if (m_httpServerInfo)
	{
		name = m_httpServerInfo->GetServerName();
	}

	return name;
}

int CHttpRequest::CheckAuth()
{
	if (m_headerInfo.m_curUserLevel == eUser_Invalid)
		return -1;
	return 0;
}

//判断当前uri是否需要权限才能访问
BOOL CHttpRequest::IsNeedAuth()
{
	string  uri = m_headerInfo.m_uri;
	if (uri == "/" || uri.empty())
	{
		return TRUE;
	}

	if (m_headerInfo.m_httpMethod == "POST" && uri.find("onvif/") != -1)
	{
		return false;//onvif命令有自己的授权机制
	}

	{
		//有几个特殊uri不需要auth就能访问
		static const char *pszArr[] =
		{
			"login.htm",
			"error.htm",
		};

		for (UINT i = 0; i < COUNT_OF(pszArr); i++)
		{
			if (uri == pszArr[i])
			{
				return FALSE;
			}
		}
	}

	static const char *pszExt[] =
	{
		//不需要auth的文件类型
		".css",
		".gif",
		".ico",	//chrome自动请求favicon.ico
		".jpg",
		".js",
		".png",
		".xml",	//ajax命令由handler来检测授权
		".wav",
	};

	BOOL needAuth = TRUE;
	{
		string  ext = HttpTool::GetUriExt(uri);
		for (UINT i = 0; i < COUNT_OF(pszExt); i++)
		{
			if (StringTool::CompareNoCase(ext, pszExt[i]) == 0)
			{
				needAuth = FALSE;
				break;
			}
		}
	}

	return needAuth;
}

int CHttpRequest::OnUnsupportedHttpMethod()
{
	string  content = StringTool::Format(
		"<html><body>Invalid/Unsupported Http Method</body></html>"
	);

	string  ack = StringTool::Format(
		"HTTP/1.1 200 OK\r\n"
		"Server: %s\r\n"
		"Content-Type: text/html;\r\n"
		"Connection: Close\r\n"
		"Cache-Control: no-cache,no-store\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		"%s"
		,
		GetServerName().c_str(),
		content.length(),
		content.c_str()
	);

	Output(ack);
	return 0;
}

//重定位到pszPage页面
int CHttpRequest::OnRedirect(const char *pszPage)
{
	string  content = StringTool::Format(
		"<html><meta http-equiv=\"refresh\" content=\"0;url=%s\"/></html>",
		pszPage
	);

	string  ack = StringTool::Format(
		"HTTP/1.1 200 OK\r\n"
		"Server: %s\r\n"
		"Content-Type: text/html;\r\n"
		"Connection: Close\r\n"
		"Cache-Control: no-cache,no-store\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		"%s"
		,
		GetServerName().c_str(),
		content.length(),
		content.c_str()
	);

	Output(ack);
	SetStatus(eHttpRequestStatus_Done);
	return 0;
}

bool CHttpRequest::IsSending()const
{
	if (m_outboxPending.GetActualDataLength() > 0)
	{
		return true;
	}

	if (m_handler && m_handler->IsSending())
	{
		return true;
	}

	return false;
}

bool CHttpRequest::IsDone()
{
	return m_eHttpRequestStatus == eHttpRequestStatus_Done && m_outboxPending.empty();
}

//当前用户是否有权执行pszAction操作
//pszUserGroup为NULL时，返回当前用户的操作权限，否则返回pszUserGroup用户组的权限.
bool CHttpRequest::IsAuthAction(const char *pszAction, const char *pszUserGroup)
{
	eUserLevel usrLevel = GetCurUserLevel();
	if (pszUserGroup)
	{
		if (StringTool::CompareNoCase(pszUserGroup, IDS_OPERATOR) == 0)
			usrLevel = eUser_Operator;
		else if (StringTool::CompareNoCase(pszUserGroup, IDS_GUEST) == 0)
			usrLevel = eUser_Guest;
		else
			usrLevel = eUser_Admin;
	}

	if (usrLevel == eUser_Admin)
		return TRUE;
	if (usrLevel == eUser_Invalid)
		return FALSE;

	ASSERT(FALSE);
	/*
	if(g_pApp)
	{
		const char *pszSection=(usrLevel == eUser_Operator)?IDS_OPERATOR:IDS_GUEST;
		BOOL bOK = g_pApp->m_ini.GetInt(pszSection,pszAction,FALSE);
		return bOK;
	}
	*/

	return TRUE;
}

//根据uri来确定handler
shared_ptr<HttpRequestHandler> CHttpRequest::CreateHandler(string  uri)
{
	shared_ptr<HttpRequestHandler> handler;
	if (mWebConfig && mWebConfig->mHttpRequestFilter)
	{
		auto obj = mWebConfig->mHttpRequestFilter->OnHttpRequest(uri);
		if (obj)
		{
			return obj;
		}
	}

	//检测特殊文件
#ifdef _CONFIG_HTTP_SNAP
	if (!handler)
	{
		static const char *arr[] =
		{
			"snap.jpg",
			"snap.cgi",
			"snapshot.cgi",
			"snapshot.jpg",
		};

		for (int i = 0; i < COUNT_OF(arr); i++)
		{
			if (uri.CompareNoCase(arr[i]) == 0)
			{
				handler = make_shared<CHttpRequestHandler_SnapFile>();
				break;
			}
		}
	}
#endif

#ifdef _CONFIG_HTTP_MJPEG_STREAM
	if (!handler)
	{
		static const char *arr[] =
		{
			"video.cgi",
			"videostream.cgi",
			"mjpeg.cgi",
		};

		for (int i = 0; i < COUNT_OF(arr); i++)
		{
			if (uri.CompareNoCase(arr[i]) == 0)
			{
				handler = make_shared<CHttpRequestHandler_MjpegStream>();
				break;
			}
		}
	}
#endif

#ifdef _CONFIG_HTTP_AUDIO
	if (!handler)
	{
		static const char *arr[] =
		{
			"audio.cgi",
		};

		for (int i = 0; i < COUNT_OF(arr); i++)
		{
			if (uri.CompareNoCase(arr[i]) == 0)
			{
				handler = make_shared<CHttpRequestHandler_AudioStream>();
				break;
			}
		}
	}
#endif

	//处理ajax,cgi
	if (!handler)
	{
		string  ext = HttpTool::GetUriExt(uri);

		if (StringTool::CompareNoCase(ext, ".xml") == 0 || ext.empty())
		{
			handler = make_shared<CHttpRequestHandler_Ajax>();
		}
		else if (StringTool::CompareNoCase(ext, ".cgi") == 0)
		{
			handler = make_shared<CHttpRequestHandler_CGI>();
		}
	}

	if (!handler)
	{
		//当作普通文件
		handler = make_shared<CHttpRequestHandler_File>();
	}

	//通用配置
	{
		handler->SetConfig(mWebConfig);
	}

	return handler;
}

int CHttpRequest::PreProcessHeader()
{
	char *request = m_headerInfo.m_request;
	int len = m_headerInfo.m_len;

	string & uri = m_headerInfo.m_uri;

	//检查是否为OPTIONS,是则转交给rtsp server处理
	if (StringTool::stristr(request, "OPTIONS") == request
		|| StringTool::stristr(request, "DESCRIBE") == request
		)
	{
		ByteBuffer box;
		box.Write(request, len + 1);//以'\0'结尾
		int ret = Transform("rtsp", box);
		return 0;
	}
	/*
	else if(
		uri.CompareNoCase("videostream.asf")==0
		|| uri.CompareNoCase("video.asf")==0
		)
	{
		ByteBuffer box;
		box.Write(request,len+1);//以'\0'结尾
		int ret=Transform("asf",box);
		return 0;
	}
	else if(m_headerInfo.m_httpMethod=="POST" && uri.Find("/onvif/")!=-1)
	{
		//string  onvifXml(m_headerInfo.m_request+m_headerInfo.m_headerLength,m_headerInfo.m_contentLength);
		const char *onvifXml=m_headerInfo.m_request+m_headerInfo.m_headerLength;
		ByteBuffer box;
		box.Write((LPVOID)onvifXml,m_headerInfo.m_contentLength);
		box.WriteByte(0);//以'\0'结尾

		Transform("onvif",box);
		return 0;
	}
	*/

	return -1;
}

//返回0表示已成功处理，不需要后续操作
int CHttpRequest::CheckProcessPostRequest(ByteBuffer& inbox)
{
	if (m_headerInfo.m_httpMethod != "POST" || m_headerInfo.m_uri.find("onvif/") != -1)
	{
		return -1;
	}

	//除了onvif POST命令，其他POST都为上传文件
	//目前有上传配置文件和升级文件

	if (!IsAuthAction("devcfg"))
	{
		OnRedirect("login.htm");
		return 0;
	}

	LPBYTE pData = inbox.GetDataPointer();
	const int cbData = inbox.GetActualDataLength();

	if (cbData >= m_headerInfo.m_headerLength + m_headerInfo.m_contentLength
		|| cbData >= 4 * 1024 - 1
		)
	{
		//接收到完整的header+content(比如上传较小的配置文件)
		//或者接收到4K字节才进行POST处理(上传firmware,是为了确保inbox中有name字段)

		Transform("post", inbox);
		return 0;

	}

	return -1;
}

void CHttpRequest::SetStatus(eHttpRequestStatus status)
{
#ifdef _DEBUG
	if (status == eHttpRequestStatus_Done)
	{
		int x = 0;
	}
#endif

	m_eHttpRequestStatus = status;
}
}
}
}
}
