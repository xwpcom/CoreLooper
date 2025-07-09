#pragma once
#include "httpformfield.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//把form data保存为文件
class HttpFormField_File :public HttpFormField
{
	SUPER(HttpFormField)
public:
	HttpFormField_File();
	virtual ~HttpFormField_File();

	int SetFolder(const std::string & folder)
	{
		mFolder = folder;
		return 0;
	}

protected:
	int Input(LPBYTE data, int dataLen);
	void OnPostFail();
	virtual void SetDataReady(bool ready = true);
	void Close();
	std::string  mFolder;
	FILE *mFile=nullptr;

	std::string  mLatitude;
	std::string  mLongitude;
	std::string  mTime;

	std::string  mFilePath;
	std::string  mFilePathTmp;//正在上传时的名称
};
}
}
}
}