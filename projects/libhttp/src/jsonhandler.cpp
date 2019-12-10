#include "stdafx.h"
#include "jsonhandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

/*
JsonHandler是从AjaxCommandHandler修改而来,改动如下
.json接口要采用代码主动注册，不再像ajax那样用宏自动注册
 自动注册好处是方便，坏处是一个库用于多个项目时，库中的ajax接口在这些项目中全部可见
 主动注册可避免此问题
*/

JsonHandler::JsonHandler()
{
	SetObjectName("JsonHandler");
}

JsonHandler::~JsonHandler()
{

}

string JsonHandler::Process(string url)
{
	ASSERT(IsMyselfThread());

	mExtraHeader.clear();

	DV("%s", url.c_str());

	string ack;

	string uri;
	NameValue params;
	HttpTool::ParseUrlParam(url, uri, params);

	/*
	if (!uri.IsEmpty())
	{
		if (uri[0] == '/')
		{
			string tmp = uri.Right(uri.GetLength() - 1);
			uri = tmp;
		}
	}

	auto handler(BaseAjaxHandler::CreateInstance(uri));
	if (handler)
	{
		handler->SetVirtualFolder(mVirtualFolder);
		handler->SetUserInfo(mUserInfo);
		handler->SetPort(mPort);

		bool hasAuth = true;
		AjaxRuntimeClass* info = (*AjaxRuntimeClass::m_mapPackClass)[uri];
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

		ack = handler->Process(params);
		mExtraHeader = handler->GetExtraHeader();
	}
	else
	{
		int errorCode = 501;//http 501错误是Unimplemented
		StringTool::AppendFormat(ack,"<Result><Error>%d</Error><Desc>Unknown command:%s</Desc></Result>",
			errorCode,
			uri.xml().c_str());
	}
	*/

	//DV("ack=[%s]", ack.c_str());
	ack = "{"
		"\"name\":\"bear\","
		"\"age\":\"38\""
		"}";

	return ack;
}

string JsonHandler::GetExtraHeader()
{
	return mExtraHeader;
}

}
}
}
}
