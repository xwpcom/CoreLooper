#include "stdafx.h"
#include "ajaxcommandhandler.h"
#include "httptool.h"
#include "libhttp/ajaxhandler.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

AjaxCommandHandler::AjaxCommandHandler()
{

}

AjaxCommandHandler::~AjaxCommandHandler()
{

}

string  AjaxCommandHandler::Process(string  url)
{
	ASSERT(IsMyselfThread());

	mExtraHeader.clear();

	string  ack;

	string  uri;
	NameValue params;
	HttpTool::ParseUrlParam(url, uri, params);

	if (!uri.empty())
	{
		if (uri[0] == '/')
		{
			string  tmp = StringTool::Right(uri, (long)uri.length() - 1);
			uri = tmp;
		}
		StringTool::Replace(uri, ".xml", "");
	}
	//ack.AppendFormat("<Result>ok for:%s</Result>", uri.xml().c_str());

	auto handler(AjaxHandler::CreateInstance(uri));
	if (handler)
	{
		handler->SetVirtualFolder(mVirtualFolder);
		handler->SetUserInfo(mUserInfo);
		handler->SetPort(mPort);
		{
			auto context = mContext.lock();
			if (context)
			{
				handler->SetContext(context);
			}
		}

		bool hasAuth = true;
		AjaxRuntimeClass *info = (*AjaxRuntimeClass::m_mapPackClass)[uri];
		if (!mUserInfo)
		{
			hasAuth = true;
		}
		else if (!info->mPermission.empty())
		{
			if (mUserInfo->HasAuth(info->mPermission.c_str()))
			{
			}
			else
			{
				hasAuth = false;
				DW("fail [%s],requires auth %s", url.c_str(), info->mPermission.c_str());
			}
		}

		if (hasAuth)
		{
			ack = handler->Process(params);
			mExtraHeader = handler->GetExtraHeader();
		}
		else
		{
			ack = StringTool::Format("<Result><Error>401</Error><Desc>Auth Fail,require permission:%s</Desc></Result>",
				info->mPermission.c_str()
			);
		}

		delete handler;
		handler = nullptr;
	}
	else
	{
		int errorCode = 501;//http 501错误是Unimplemented
		StringTool::AppendFormat(ack, "<Result><Error>%d</Error><Desc>Unknown command:%s</Desc></Result>",
			errorCode,
			StringTool::xml(uri).c_str());
	}

	DV("len=%d,%s", ack.length(),ack.c_str());
	return ack;
}

string  AjaxCommandHandler::GetExtraHeader()
{
	return mExtraHeader;
}

}
}
}
}
