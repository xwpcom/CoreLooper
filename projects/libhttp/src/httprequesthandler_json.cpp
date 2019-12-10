#include "stdafx.h"
#include "httprequesthandler_json.h"
#include "jsonhandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpRequestHandler_Json::HttpRequestHandler_Json()
{
}

HttpRequestHandler_Json::~HttpRequestHandler_Json()
{
}

int HttpRequestHandler_Json::Process()
{
	//json命令总是一次性解决的
	ASSERT(FALSE);
	return 0;
}

int HttpRequestHandler_Json::Start(tagHttpHeaderInfo* headerInfo)
{
	HttpRequestHandler::Start(headerInfo);
	SetStatus(eHttpHandlerStatus_Processing);

	string ack;
	string xml;

	auto json = mWebConfig->mJsonHandler.lock();
	if (json)
	{
		auto userInfo = make_shared<UserInfo>();
		userInfo->SetUserMan(mWebConfig->mUserMan);
		userInfo->SetUserPassword(this->GetUserName(), this->GetPassword());
		json->SetVirtualFolder(mWebConfig->mVirtualFolder);
		json->SetUserInfo(userInfo);

		{
			auto localAddr = headerInfo->mLocalAddr;
			auto pos = localAddr.find(':');
			if (pos != -1)
			{
				json->SetPort(atoi(localAddr.c_str() + pos + 1));
			}
		}

		xml = json->Process(headerInfo->mUrl);
		DWORD len = xml.length();

		HttpAckHeader httpAckHeader;
		httpAckHeader.SetStatusCode("200 OK");
#ifndef _MINI_HTTP
		httpAckHeader.SetConnection("Keep-Alive");
#endif
		httpAckHeader.SetCacheControl("no-cache,no-store");//,must-revalidate");
		httpAckHeader.SetContentType("application/json;charset=UTF-8");

		if (len > 0)
		{
			if (xml.at(0) != '<')
			{
				//httpAckHeader.SetContentType("text/plain;charset=UTF-8");
			}
		}

		httpAckHeader.SetContentLength(len);

		ack = httpAckHeader.ack(FALSE);
		ack += json->GetExtraHeader();
		ack += "\r\n";
	}
	else
	{
		ASSERT(FALSE);
	}

	Output(ack);
	Output(xml);

#ifdef _MSC_VER_DEBUG
	{
		static int idx = -1;
		++idx;
		string filePath=StringTool::Format("g:/test/device/%04d_ack.json", idx);

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
