#include "stdafx.h"
#include "PostHandler_UploadPicture.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

PostHandler_UploadPicture::PostHandler_UploadPicture()
{

}

PostHandler_UploadPicture::~PostHandler_UploadPicture()
{

}

shared_ptr<HttpFormField> PostHandler_UploadPicture::CreateField(const string & fieldName)
{
	NameValue& param = mHeader->GetHeader().mUrlParams;
	auto obj = make_shared<HttpFormField_File>();
	string  folder = StringTool::Format("%s/pictures/%s", mWebConfig->mMediaRootPath.c_str(), param.GetString("uid").c_str());
	obj->SetFolder(folder);
	return obj;
}
}
}
}
}
