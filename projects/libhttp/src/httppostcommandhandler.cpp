#include "stdafx.h"
#include "httppostcommandhandler.h"
#include "libhttp/postjsonmanager.h"

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
	if (mCurrentField)
	{
		HttpFormField::eResult ret = mCurrentField->Input(inbox);
		return ret;
	}

	{
		auto data = inbox.data();
		auto bytes = inbox.length();
		
		int eat = MAX(0,mContentLength - mInboxBody.length());
		if (eat > 0)
		{
			mInboxBody.Write(data, eat);
			mInboxBody.MakeSureEndWithNull();
			inbox.Eat(eat);

			if (mInboxBody.length() == mContentLength)
			{
				data = mInboxBody.data();
				bytes = mInboxBody.length();

				{
					auto obj = _Object(PostJsonManager, "PostJsonManager");
					if (obj)
					{
						auto& header=mHeader->GetHeader();
						string name = header.mUri;
						auto handler = obj->CreatePostJsonHandler(name);
						if (handler)
						{
							obj->AddChild(handler);

							DynamicJsonBuffer jBuffer;
							auto& json = jBuffer.parseObject(mInboxBody.data());
							
							string ackJson = handler->ProcessJson(json);
							if (!ackJson.empty())
							{
								mAck = StringTool::Format(
									"HTTP/1.1 200\r\n"
									"Content-Type: application/json;charset=UTF-8\r\n"
									"Content-Length: %d\r\n"
									"\r\n"
									"%s"
									, (int)ackJson.length()
									, ackJson.c_str()
								);
							}

							handler->Destroy();
						}
					}
				}

				if (mAck.empty())
				{
					string jsonText = "{\"error\":-1,\"desc\":\"no found json handler\"}";
					mAck = StringTool::Format(
						"HTTP/1.1 200\r\n"
						"Content-Type: application/json;charset=UTF-8\r\n"
						"Content-Length: %d\r\n"
						"\r\n"
						"%s"
						, (int)jsonText.length()
						, jsonText.c_str()
					);
				}

				return HttpFormField::eResult_Finish;
			}
		}
	}

	return HttpFormField::eResult_NeedMoreData;
}

string HttpPostCommandHandler::GetAck()
{
	return mAck;
}

}
}
}
}
