#include "stdafx.h"
#include "FileUploadManager.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

static const char* TAG = "FileUploadManager";

FileUploadManager::FileUploadManager()
{
	SetObjectName("FileUploadManager");
}

void FileUploadManager::OnCreate()
{
	__super::OnCreate();
}

void FileUploadManager::OnUploadFile(const string& filePath, Bundle& params, string& httpAck)
{
	LogV(TAG, "%s,filePath=[%s]",__func__,filePath.c_str());
}

}
}
}
}
