#include "stdafx.h"
#include "HttpPostHandler.h"
#include "httprequest.h"
#include "httpheader.h"
#include "httppostparser.h"
#include "httpformfield.h"
#include "httppostcommandhandler.h"
#include "httptool.h"
#include "PostHandler_UploadPicture.h"
#include "PostHandler_UploadVideo.h"

#define CRLF2		"\r\n\r\n"
#define CRLF2LEN	4

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpPostHandler::HttpPostHandler()
{
	mState = eState_WaitHttpHeader;
}

HttpPostHandler::~HttpPostHandler()
{
	if (mCommandHander)
	{
		mCommandHander->OnFinishRecvData(-1);
		mCommandHander = nullptr;
	}
}

int HttpPostHandler::Input(ByteBuffer& inbox)
{
	ASSERT(mWebConfig);
	//ASSERT(mHttpRequest);

	inbox.MakeSureEndWithNull();//方便字符串解析
	//*
	if (mState == eState_Done)
	{
		ASSERT(FALSE);
		return -1;
	}
	//*/

	while (1)
	{
		if (inbox.GetActualDataLength() == 0)
		{
			return 0;
		}

		switch (mState)
		{
		case eState_WaitHttpHeader:
		{
			const char *psz = (const char *)inbox.GetDataPointer();
			int len = inbox.GetActualDataLength();
			if (!psz)
			{
				return 0;
			}

			const char *headerEnd = strstr(psz, CRLF2);
			if (!headerEnd)
			{
				return 0;
			}

			int bytes = (int)(headerEnd - psz + CRLF2LEN);
			string  ackHeader(psz, bytes);

			mHeader = make_shared<HttpHeader>();
			mHeader->Parse(ackHeader);
			tagHttpHeader& header = mHeader->GetHeader();
			ASSERT(!mCommandHander);

			mCommandHander = CreatePostHandler(mHeader);

			inbox.Eat(bytes);
			SwitchState(eState_WaitFormDataHeader);
			break;
		}

		case eState_WaitFormDataHeader:
		{
			ASSERT(mCommandHander);

			const char *psz = (const char *)inbox.GetDataPointer();
			int leftBytes = inbox.GetActualDataLength();
			if (leftBytes == 0)
			{
				return 0;
			}

			const char *end = strstr(psz, CRLF2);
			if (!end)
			{
				//检测是否遇到最后一个boundary
				if (psz[0] == '\r')
				{
					string  boundary = mHeader->GetHeader().mFields.GetString("boundary");
					string  key = "\r\n--" + boundary + "--\r\n";
					int keyBytes = (int)key.length();
					if (strncmp(psz, key.c_str(), keyBytes) == 0)
					{
						inbox.Eat(keyBytes);
						int error = 0;
						mCommandHander->OnFinishRecvData(error);
						mCommandHander = nullptr;
						SwitchState(eState_Done);
						return 0;
					}
				}

				return 0;
			}

			/*
			样本
			------WebKitFormBoundaryUxnYqUOngu5937wp
			Content-Disposition: form-data; name="firmware"; filename="ffmpeg.h264.extradata.bin"
			Content-Type: text/plain
			*/
			auto header = string(psz, end - psz);

			string  fieldName = HttpTool::Mid(header, "name=\"", "\"");
			string  filename = HttpTool::Mid(header, "filename=\"", "\"");
			string  range = HttpTool::Mid(header, "\r\nrange: ", "-");
			int rangeStart = atoi(range.c_str());
			rangeStart = MAX(0, rangeStart);

			DV("field=[%s],filename=[%s]", fieldName.c_str(), filename.c_str());
			mCommandHander->BeginField(fieldName, rangeStart);

			int bytes = (int)(end + CRLF2LEN - psz);
			inbox.Eat(bytes);
			SwitchState(eState_WaitFormDataBody);
			break;
		}
		case eState_WaitFormDataBody:
		{
			HttpFormField::eResult ret = mCommandHander->Input(inbox);
			if (ret == HttpFormField::eResult_Finish)
			{
				mCommandHander->EndField();

				SwitchState(eState_WaitFormDataHeader);
				break;
			}
			else if (ret == HttpFormField::eResult_NeedMoreData)
			{
				return 0;
			}
			else if (ret == HttpFormField::eResult_Error)
			{
				return 400;//http bad request
			}

			break;
		}
		}
	}

	return 0;
}

bool HttpPostHandler::IsDone()
{
	return mState == eState_Done;
}

void HttpPostHandler::SwitchState(HttpPostHandler::eState state)
{
	mState = state;
}

shared_ptr<HttpPostCommandHandler> HttpPostHandler::CreatePostHandler(shared_ptr<HttpHeader> header)
{
	const string & cmd = header->GetHeader().mUri;
	if (StringTool::CompareNoCase(cmd, "UploadPicture.cgi") == 0)
	{
		auto obj = make_shared<PostHandler_UploadPicture>();
		obj->Init(mHeader);
		obj->SetConfig(mWebConfig);
		return obj;
	}
	else if (StringTool::CompareNoCase(cmd, "UploadVideo.cgi") == 0)
	{
		auto obj = make_shared<PostHandler_UploadVideo>();
		obj->Init(mHeader);
		obj->SetConfig(mWebConfig);
		return obj;
	}



	auto obj = make_shared<HttpPostCommandHandler>();
	obj->Init(mHeader);
	obj->SetConfig(mWebConfig);
	return obj;
}

}
}
}
}