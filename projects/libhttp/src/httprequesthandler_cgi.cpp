#include "stdafx.h"
#include "httprequesthandler_cgi.h"
#include "httptool.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpRequestHandler_CGI::HttpRequestHandler_CGI()
{
}

HttpRequestHandler_CGI::~HttpRequestHandler_CGI()
{
}

int HttpRequestHandler_CGI::Process()
{
	//总是一次性解决的
	ASSERT(FALSE);
	return 0;
}

int HttpRequestHandler_CGI::Start(tagHttpHeaderInfo *headerInfo)
{
	HttpRequestHandler::Start(headerInfo);
	SetStatus(eHttpHandlerStatus_Processing);

	string  cgiAck;
	ProcessCgi(cgiAck);

	HttpAckHeader httpAckHeader;
	httpAckHeader.SetStatusCode("200 OK");
	httpAckHeader.SetCacheControl("no-cache,no-store,must-revalidate");
	httpAckHeader.SetContentLength((int)cgiAck.length());
	httpAckHeader.SetContentType("text/plain");

	string  ack = httpAckHeader.ack();
	ack += cgiAck;

	/*
	ack.Format(
		"HTTP/1.1 200 OK\r\n"
		"Cache-Control: no-cache,no-store,must-revalidate\r\n"
		"Content-Length: %lu\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"%s"
		,
		cgiAck.length(),
		cgiAck.c_str()
	);
	//*/

	Output(ack);

	SetStatus(eHttpHandlerStatus_Done);
	return 0;
}

//XiongWanPing 2010.10.15
//
int HttpRequestHandler_CGI::ProcessCgi(string & ack)
{
	string  szUri = m_headerInfo->m_uri;
	ack = "";

	ack = "[error]:badauth\r\n";
	if (StringTool::CompareNoCase(szUri, "/reboot.cgi") == 0)
	{
		BOOL CanConfig = IsAuthAction("devcfg");
		if (CanConfig)
		{
			DF("reboot.cgi reboot");
			ASSERT(FALSE);
			//g_pApp->SetDelayReboot();
			ack = "[ok]\r\n";
		}
	}
	else if (StringTool::CompareNoCase(szUri, "/restore_factory.cgi") == 0)
	{
		BOOL CanConfig = IsAuthAction("devcfg");
		if (CanConfig)
		{
			DF("restore_factory.cgi reboot");
			//g_pApp->m_ini.SetInt(IDS_APP,"reset",1);
			//g_pApp->RestoreFactoryConfig();
			//g_pApp->SetDelayReboot();
			ASSERT(FALSE);
			ack = "[ok]\r\n";
		}
	}
	else if (StringTool::CompareNoCase(szUri, "/set_devname.cgi") == 0)
	{
		BOOL CanConfig = IsAuthAction("devcfg");
		if (CanConfig)
		{
			if (IsParamExists("devname"))
			{
				string  name = GetParamString("devname");
				//g_pApp->m_ini.SetString(IDS_DEV,"Name",name);
				ASSERT(FALSE);
			}

			ack = "[ok]\r\n";
		}
	}
	else
	{
		ack = StringTool::Format("[error]:unknown cgi cmd:[%s]\r\n", szUri.c_str());
	}

	return 0;
}

}
}
}
}