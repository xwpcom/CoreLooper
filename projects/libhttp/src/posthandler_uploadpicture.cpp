#include "stdafx.h"
#include "PostHandler_UploadPicture.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "postHandler";

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

/*
XiongWanPing 2021.07.12
t21 camera上传图片
参数
uid

约定保存位置
mediaRoot/pictures/uid/yyyy-mm/dd/yyyy-mm-dd hhMMss.jpg
*/

PostHandler_UploadFile::PostHandler_UploadFile()
{
}

PostHandler_UploadFile::~PostHandler_UploadFile()
{
}

shared_ptr<HttpFormField> PostHandler_UploadFile::CreateField(const string& fieldName)
{
	/*
	if (fieldName != "file")
	{
		return make_shared<HttpFormField>();
	}
	*/

	string  folder;
	auto obj = make_shared<HttpFormField_File>();

	NameValue& params= mHeader->GetHeader().mUrlParams;
	auto tag = params.GetString("tag");
	auto uid = params.GetString("uid");

	if (tag == "cameraSnap")
	{
		obj->params().Set("tag", tag);
		obj->params().Set("uid", uid);

		auto date= params.GetString("date");// yyyymmdd
		//auto time = params.GetString("time");//hhMMss
		LogV(TAG, "uid=%s", uid.c_str());

		auto dateFolder = StringTool::Format("%s-%s-%s"
			, date.substr(0,4).c_str() 
			,date.substr(4, 2).c_str()
			, date.substr(6, 2).c_str()
		);
		folder = StringTool::Format("%s/%s/%s"
			, mWebConfig->mMediaRootPath.c_str()
			,uid.c_str()
			,dateFolder.c_str()
		);

		auto fileName = params.GetString("fileName");
		obj->SetFileName(fileName);
	}
	else
	{
		folder = StringTool::Format("%s/pictures/%s", mWebConfig->mMediaRootPath.c_str(), uid.c_str());
	}

	obj->SetFolder(folder);
	return obj;
}


}
}
}
}
