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

/*
2022.06.17
todo:这个要做成可扩展的
*/
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
		if (date.length() <  8)
		{
			LogW(TAG, "invalid date[%s]",date.c_str());
			return nullptr;
		}

		/*
		date为空时,date.substr(0,4)运行不会报错
		而date.substr(4, 2)会导致crash
		*/

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
	else if (tag == "mcuLog")
	{
		folder = StringTool::Format("C:/iot/web/firmware/mcu/mcuLog/%s", uid.c_str());
		auto fileName = params.GetString("fileName");
		obj->SetFileName(fileName);
	}
	else if (tag == "file")
	{
		folder = StringTool::Format("C:/iot/file/%s", uid.c_str());
		auto fileName = params.GetString("fileName");
		obj->SetFileName(fileName);
	}
	else if (tag == "rtspSnap")
	{
		obj->params().Set("tag", tag);
		obj->params().Set("uid", uid);

		folder = StringTool::Format("C:/iot/rtspSnap/%s", uid.c_str());
		auto fileName = params.GetString("fileName");
		obj->SetFileName(fileName);
	}
	else if (tag == "gxCsv")
	{
		obj->params().Set("tag", tag);
		auto id = params.GetString("id");
		obj->params().Set("id", id);

		folder = StringTool::Format("C:/iot/gx");
		auto t = tagTimeMs::now();
		auto fileName = StringTool::Format("%d%02d%02d_%02d%02d%02d.csv",t.year,t.month,t.day,t.hour,t.minute,t.second);
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
