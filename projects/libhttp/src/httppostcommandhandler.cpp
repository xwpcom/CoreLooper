#include "stdafx.h"
#include "httppostcommandhandler.h"

using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpPostCommandHandler::HttpPostCommandHandler()
{
}

HttpPostCommandHandler::~HttpPostCommandHandler()
{

}

int HttpPostCommandHandler::Init(shared_ptr<HttpHeader> header)
{
	mHeader = header;
	return 0;
}

void HttpPostCommandHandler::OnFinishRecvData(int error)
{
	//DV("OnFinishRecvData,error=%d",error);

	if (error == 0)
	{
#ifdef _DEBUG
		for (auto iter = mFields.begin(); iter != mFields.end(); ++iter)
		{
			ASSERT((*iter)->IsDataReady());
		}
#endif
	}
	else
	{
		for (auto iter = mFields.begin(); iter != mFields.end(); ++iter)
		{
			(*iter)->OnPostFail();
		}
	}
}

//子类可根据fieldName来创建HttpFormField子类,进行不同的处理
shared_ptr<HttpFormField> HttpPostCommandHandler::CreateField(const string & fieldName)
{
	auto obj = make_shared<HttpFormField>();
	obj->SetConfig(mWebConfig);
	return obj;
}

int HttpPostCommandHandler::BeginField(const string & fieldName, int rangeStart)
{
	ASSERT(!mCurrentField);

	mCurrentField = CreateField(fieldName);
	ASSERT(mCurrentField);

	mCurrentField->InitField(mHeader, fieldName, rangeStart);

	return 0;
}

int HttpPostCommandHandler::EndField()
{
	if (mCurrentField)
	{
		mCurrentField->SetDataReady();
		mFields.push_back(mCurrentField);
		mCurrentField = nullptr;
	}
	else
	{
		ASSERT(FALSE);
		return -1;
	}

	return 0;
}

HttpFormField::eResult HttpPostCommandHandler::Input(ByteBuffer& inbox)
{
	ASSERT(mCurrentField);

	HttpFormField::eResult ret = mCurrentField->Input(inbox);
	return ret;
}

}
}
}
}
