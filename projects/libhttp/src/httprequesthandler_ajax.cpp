#include "stdafx.h"
#include "httprequesthandler_ajax.h"
//#include "httpclient.h"
#include "libhttp/ajaxcommandhandler.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpRequestHandler_Ajax::HttpRequestHandler_Ajax()
{
}

HttpRequestHandler_Ajax::~HttpRequestHandler_Ajax()
{
}

int HttpRequestHandler_Ajax::Process()
{
	//ajax命令总是一次性解决的
	ASSERT(FALSE);
	return 0;
}

int HttpRequestHandler_Ajax::Start(tagHttpHeaderInfo *headerInfo)
{
	HttpRequestHandler::Start(headerInfo);
	SetStatus(eHttpHandlerStatus_Processing);

	string  ack;
	string  xml;

	auto ajax = mWebConfig->mAjaxCommandHandler.lock();
	if (ajax)
	{
		auto userInfo = make_shared<UserInfo>();
		userInfo->SetUserMan(mWebConfig->mUserMan);
		userInfo->SetUserPassword(this->GetUserName(), this->GetPassword());
		ajax->SetVirtualFolder(mWebConfig->mVirtualFolder);
		ajax->SetUserInfo(userInfo);

		{
			auto localAddr = headerInfo->mLocalAddr;
			auto pos = localAddr.find(':');
			if (pos != -1)
			{
				ajax->SetPort(atoi(localAddr.c_str() + pos + 1));
			}
		}

		xml =
			//"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"+ 
			ajax->Process(headerInfo->mUrl);// , headerInfo->m_urlParam);
		DWORD len = (int)xml.length();

		HttpAckHeader httpAckHeader;
		httpAckHeader.SetStatusCode("200 OK");
#ifndef _MINI_HTTP
		httpAckHeader.SetConnection("Keep-Alive");
#endif
		httpAckHeader.SetCacheControl("no-cache,no-store");//,must-revalidate");
		httpAckHeader.SetContentType("text/xml;charset=UTF-8");

		if (len > 0)
		{
			if (xml[0] != '<')
			{
				httpAckHeader.SetContentType("text/plain;charset=UTF-8");
			}
		}

		httpAckHeader.SetContentLength(len);

		ack = httpAckHeader.ack(FALSE);
		ack += ajax->GetExtraHeader();
		ack += "\r\n";
	}
	else
	{
		ASSERT(FALSE);
	}

	/*
	ack.Format(
		"HTTP/1.1 200 OK\r\n"
		//"Server: %s\r\n"
		"Cache-Control: no-cache,no-store,must-revalidate\r\n"
		"%s"//extra header
		"Content-Length: %lu\r\n"
		"Content-Type: text/xml;charset=UTF-8\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n"
		,
		m_szXmlAckExtraHeader.IsEmpty()?"":m_szXmlAckExtraHeader.c_str(),
		len
		);
	//*/

	Output(ack);
	Output(xml);

#ifdef _MSC_VER_DEBUG
	{
		static int idx = -1;
		++idx;
		string  filePath = StringTool::Format("g:/test/device/%04d_ack.bin", idx);

		File::Dump(ack + xml, filePath.c_str());
	}
#endif


	SetStatus(eHttpHandlerStatus_Done);
	return 0;
}

}
}
}
}