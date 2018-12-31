#include "stdafx.h"
#include "PostHandler_UploadVideo.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

PostHandler_UploadVideo::PostHandler_UploadVideo()
{

}

PostHandler_UploadVideo::~PostHandler_UploadVideo()
{

}

shared_ptr<HttpFormField> PostHandler_UploadVideo::CreateField(const string & fieldName)
{
	NameValue& param = mHeader->GetHeader().mUrlParams;
	auto obj = make_shared<HttpFormField_File>();
	string  folder = StringTool::Format("%s/videos/%s", mWebConfig->mMediaRootPath.c_str(), param.GetString("uid").c_str());
	obj->SetFolder(folder);
	return obj;
}
}
}
}
}
