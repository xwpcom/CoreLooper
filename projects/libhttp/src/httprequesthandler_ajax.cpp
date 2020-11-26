#include "stdafx.h"
#include "httprequesthandler_ajax.h"
#include "libhttp/ajaxcommandhandler.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "Http_Ajax";

HttpRequestHandler_Ajax::HttpRequestHandler_Ajax()
{
}

HttpRequestHandler_Ajax::~HttpRequestHandler_Ajax()
{
}

int HttpRequestHandler_Ajax::Process()
{
	LogV(TAG, "(%p)%s",this,__func__);
	//ASSERT(FALSE);
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

		xml =ajax->Process(headerInfo->mUrl);
		auto len = (int)xml.length();

		HttpAckHeader httpAckHeader;
		httpAckHeader.SetStatusCode("200 OK");
		httpAckHeader.SetConnection("Keep-Alive");
		httpAckHeader.SetCacheControl("no-cache,no-store");
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
		ack=StringTool::Format(
			"HTTP/1.1 501\r\n"
			"Cache-Control: no-cache,no-store,must-revalidate\r\n"
			"Connection: Close\r\n"
			"\r\n"
		);
		xml.clear();
	}

	Output(ack);
	Output(xml);

	SetStatus(eHttpHandlerStatus_Done);
	return 0;
}

}
}
}
}