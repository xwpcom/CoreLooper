#pragma once
//XiongWanPing 2017.07.19
#define _MINI_HTTP	//mcu单片机流量较贵，所以精简一下

#include "auth/userinfo.h"
#include "auth/userman.h"
//#include "base/keyvalue.h"
#include "base/namevalue.h"
//max-age以秒为单位
#ifdef _DEBUG
#define FILE_CACHE_MAX_AGE	"Cache-Control: max-age=5\r\n"		
#else
#define FILE_CACHE_MAX_AGE	"Cache-Control: max-age=60\r\n"
#endif

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

enum eHttpHandlerStatus
{
	eHttpHandlerStatus_Idle,
	eHttpHandlerStatus_Processing,
	eHttpHandlerStatus_Done,
};

struct HTTP_EXPORT tagHttpHeaderInfo
{
	tagHttpHeaderInfo()
	{
		Reset();
	}

	void Reset()
	{
		m_request = NULL;
		m_len = 0;

		m_headerLength = 0;
		m_contentLength = 0;

		m_curUserLevel = eUser_Invalid;

		m_peerAddr.clear();
		mLocalAddr.clear();
		m_httpMethod.clear();
		m_uri.clear();
		m_szReferer.clear();
		m_urlParam.Empty();
		m_cookies.Empty();

		m_if_modified_since = -1;
		m_range = 0;
		m_rangeEnd = 0;
	}

	char		*m_request=nullptr;	//指向http request的临时buf,没有自分配内存
	UINT		m_len=0;			//m_request长度

	int			m_headerLength=0;	//http头长度(以\r\n\r\n结尾)
	int			m_contentLength=0;	//http头后面的附加数据长度,是放在\r\n\r\n后面的数据长度.

	eUserLevel	m_curUserLevel;

	std::string 	m_peerAddr;			//对方ip地址，供添加日志使用
	std::string		mLocalAddr;
	std::string    m_httpMethod;		//GET,POST
	std::string 	mUrl;				//包括参数,比如AddUser.xml?user=admin&password=admin&usr=xwp&pwd=xxx
	std::string 	m_uri;				//Uniform Resource Identifier:http网页名称,可能是文件名,cgi,或者符号名
	std::string 	m_szReferer;
	NameValue	m_urlParam;
	NameValue	m_cookies;

	time_t		m_if_modified_since=0;
	size_t		m_range = 0;	//断点续传起点
	size_t		m_rangeEnd = 0;	//断点续传终点(为0表示没有指定),目前并没用到
};


//注意要和CHttpClient::GetLastErrorDesc保持同步
enum eHttpError
{
#undef ITEM
#define ITEM(x)	x
	ITEM(eHttpError_None),
	ITEM(eHttpError_No_Found_Page),
	ITEM(eHttpError_Unknown),
	ITEM(eHttpError_Invalid_User_Password),
	ITEM(eHttpError_Invalid_Http_Cmd),
	ITEM(eHttpError_No_Auth),	//权限不够
	ITEM(eHttpError_OK),//操作成功完成
	ITEM(eHttpError_Not_Exist),
	ITEM(eHttpError_Invalid_XML_Request),
	ITEM(eHttpError_Invalid_Value),
	ITEM(eHttpError_User_Already_Exist),
#undef ITEM
};

struct tagOnvifHandlerInfo
{
	SOCKET		m_sock;
	std::string m_request;//完整的onvif xml request
};

}
}
}
}