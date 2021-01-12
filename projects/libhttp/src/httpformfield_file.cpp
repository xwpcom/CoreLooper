#include "stdafx.h"
#include "httpformfield_file.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
using namespace FileSystem;

namespace Net {
namespace Http {

static const char* TAG = "HttpFormField_File";

HttpFormField_File::HttpFormField_File()
{
	mFile = nullptr;
}

HttpFormField_File::~HttpFormField_File()
{
	Close();
}

void HttpFormField_File::Close()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;

		if (mDataReady)
		{
			if (File::FileExists(mFilePath.c_str()))
			{
				File::DeleteFile(mFilePath.c_str());
			}

			File::rename(mFilePathTmp.c_str(), mFilePath.c_str());
		}
		else
		{
			//自动删除无效的数据
		}
	}
}

int HttpFormField_File::Input(LPBYTE data, int dataLen)
{
	if (!mFile)
	{
		//mFieldName中可能也带有子目录

		string  fullFilePath = mFolder + "/" + mFieldName;
		File::CreateFolderForFile(fullFilePath.c_str());
		mFilePath = fullFilePath;
		mFilePathTmp = fullFilePath + ".tmp";
		if (mRangeStart == 0)
		{
			mFile = fopen(mFilePathTmp.c_str(), "wb");
		}
		else
		{
			mFile = fopen(mFilePathTmp.c_str(), "a+b");
		}
	}

	if (mFile)
	{
		static int idx = -1;
		++idx;
		int ret = dataLen;
		//if (idx == 0)
		{
			ret = (int)fwrite(data, 1, dataLen, mFile);
		}

		if (ret == dataLen)
		{
			return 0;
		}
		else
		{
			LogW(TAG,"fail fwrite,dataLen=%d,ret=%d,error=%d", dataLen, ret, errno);
		}
	}
	else
	{
		int x = 0;
	}

	return -1;
}

void HttpFormField_File::SetDataReady(bool ready)
{
	__super::SetDataReady(ready);

	if (ready)
	{
		Close();
	}
}

void HttpFormField_File::OnPostFail()
{
	//不要删除已下载的部分文件，可用于支持断点继传
}

}
}
}
}
