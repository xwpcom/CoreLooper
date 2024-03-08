#include "stdafx.h"
#include "httppostcommandhandler.h"
#include "libhttp/postjsonmanager.h"

using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "HttpPostCommandHandler";

HttpPostCommandHandler::HttpPostCommandHandler()
{
	mInboxBody.SetBufferSize(16 * 1024, 1024 * 1024 * 16);
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
		eat = MIN(bytes, eat);
		if (eat > 0)
		{
			mInboxBody.Write(data, eat);
			mInboxBody.MakeSureEndWithNull();
			inbox.Eat(eat);

			if (mInboxBody.length() == mContentLength)
			{
				data = mInboxBody.data();
				bytes = mInboxBody.length();

				auto& header = mHeader->GetHeader();
				auto& name = header.mUri;
				{
#ifdef _CONFIG_ANDROID
					/*
					2021.03.04
					采用目前最新的AndroidStudio4.1.2
					在liblift.so中定义的PostJsonManager子类,
					在libhtt.so中能搜索到Handler,但dynamic_pointer_cast到PostJsonManager时返回nullptr
					vs2019没有此问题
					感觉是as的bug?

					临时解决办法:c style cast
					*/
					//LogV(TAG, "PostJsonManager=%p",obj?obj.get():nullptr);
					auto obj2 = _Object(Handler, "PostJsonManager");
					//LogV(TAG, "Handler.PostJsonManager=%p", obj2 ? obj2.get() : nullptr);
					PostJsonManager* obj = nullptr;
					if (obj2)
					{
						obj = (PostJsonManager*)obj2.get();
					}
#else
					auto obj = _Object(PostJsonManager, "PostJsonManager");
#endif					
					if (obj)
					{
						//LogV(TAG, "%s#3", __func__);
						//LogV(TAG, "try handler json:%s", name.c_str());
						auto handler = obj->CreatePostJsonHandler(name);
						if (handler)
						{
							handler->setHttpRequest(mHttpRequest);

							//LogV(TAG, "%s#4", __func__);
							obj->AddChild(handler);

						#ifdef _MSC_VER_DEBUGx
							if (mInboxBody.length() > 100 * 1024)
							{
								File::Dump(mInboxBody, "d:/test.bin");
							}
						#endif

							DynamicJsonBuffer jBuffer;
							auto& json = jBuffer.parseObject(mInboxBody.data());
							auto ok = json.success();

							string ackJson = handler->ProcessJson(json);
							if (!ackJson.empty())
							{
								mAck = StringTool::Format(
									"HTTP/1.1 200 OK\r\n"
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
					DynamicJsonBuffer jBuffer;
					auto& json = jBuffer.createObject();
					{
						json["error"] = -1;
						json["desc"] = StringTool::Format("no found json handler(%s)",name.c_str());
					}
					string jsonText;
					json.printTo(jsonText);

					mAck = StringTool::Format(
						"HTTP/1.1 200 OK\r\n"
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
